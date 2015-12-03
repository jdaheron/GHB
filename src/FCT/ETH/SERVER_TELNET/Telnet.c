/**************************************************************************************************/
/*																								  */
/* Telnet protocol																				  */
/*																								  */
/**************************************************************************************************/


/*--------------------------------------------------------------------------------------------------
	INCLUDES
--------------------------------------------------------------------------------------------------*/

#include <ETH/lwip-1.4.1/src/include/lwip/debug.h>
#include <ETH/lwip-1.4.1/src/include/lwip/stats.h>
#include <ETH/lwip-1.4.1/src/include/lwip/tcp.h>
#include <ETH/SERVER_TELNET/Telnet.h>
#include <string.h>
#include "../../Terminal.h"


/*--------------------------------------------------------------------------------------------------
	PRIVATE DEFINE
--------------------------------------------------------------------------------------------------*/

#define LogId			"ETH_TELNET"


#define usprintf		_sprintf
#define UARTprintf		_printf

#define TELNET_IAC		255
#define TELNET_WILL		251
#define TELNET_WONT		252
#define TELNET_DO		253
#define TELNET_DONT		254

#define IDLE_TIMEOUT	30		// 30 seconds

#define TELNET_FLAG_CLOSE_CONNECTION	(1<<0)

#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_WILL   2
#define STATE_WONT   3
#define STATE_DO     4
#define STATE_DONT   5
#define STATE_CLOSE  6


/*--------------------------------------------------------------------------------------------------
	PRIVATE TYPEDEF
--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------
	PRIVATE DATA DECLARATION
--------------------------------------------------------------------------------------------------*/

struct telnet_state
{
	char	*data_out;
	char	cmd_buffer[80];
	u16_t	left;
	u16_t	timeout;
	u16_t	flags;
	u16_t	state;
};

static uint8_t Telnet_NbActiveconnection;

char tstr[1024];
static char* Telnet_Prompt;
static char* Telnet_Welcom;
extern char VERSION_SW[];



/*--------------------------------------------------------------------------------------------------
	FUNCTIONS DEFINITIONS
--------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------*/
uint8_t Telnet_GetNbActiveConnection(void)
{
	return Telnet_NbActiveconnection;
}


/*------------------------------------------------------------------------------------------------*/
void cmd_parser(struct telnet_state *hs)
{
	Terminal_Parser(hs->cmd_buffer, tstr, 1024);
	if (strlen(tstr) > 0)
	{
		hs->data_out = tstr;
		hs->left = strlen(hs->data_out);
	}

	hs->cmd_buffer[0] = 0;

	return;
}


/*------------------------------------------------------------------------------------------------*/
static void conn_err(void *arg, err_t err)
{
	struct http_state *hs;
	LWIP_UNUSED_ARG(err);
	hs = arg;
	mem_free(hs);
}


/*------------------------------------------------------------------------------------------------*/
static void close_conn(struct tcp_pcb *pcb, struct telnet_state *hs)
{
	_CONSOLE( LogId, "Close connection from %d.%d.%d.%d:%d\n",
			(pcb->remote_ip.addr >> 0) & 0xff,
			(pcb->remote_ip.addr >> 8) & 0xff,
			(pcb->remote_ip.addr >> 16) & 0xff,
			(pcb->remote_ip.addr >> 24) & 0xff,
			(pcb->remote_port) & 0xff
	);

	tcp_arg(pcb, NULL);
	tcp_sent(pcb, NULL);
	tcp_recv(pcb, NULL);
	mem_free(hs);
	tcp_close(pcb);

	if (Telnet_NbActiveconnection > 0)
	{
		Telnet_NbActiveconnection--;
	}
}


/*------------------------------------------------------------------------------------------------*/
static void send_data(struct tcp_pcb *pcb, struct telnet_state *hs)
{
	err_t err;
	u16_t len;

	/* We cannot send more data than space available in the send
	buffer. */
	if (tcp_sndbuf(pcb) < hs->left) {
		len = tcp_sndbuf(pcb);
	} else {
		len = hs->left;
	}

	do {
		err = tcp_write(pcb, hs->data_out, len, 0);
		if (err == ERR_MEM) {
			len /= 2;
		}
	} while (err == ERR_MEM && len > 1);

	if (err == ERR_OK) {
		hs->data_out += len;
		hs->left -= len;
	}
}


/*------------------------------------------------------------------------------------------------*/
static err_t telnet_poll(void *arg, struct tcp_pcb *pcb)
{
	struct telnet_state *hs;

	hs = arg;

	//UARTprintf("Poll\n");

	if (hs == NULL) {
		tcp_abort(pcb);
		return ERR_ABRT;
	} else {

		if (hs->flags & TELNET_FLAG_CLOSE_CONNECTION) {
			_CONSOLE( LogId, "closing connection in poll\n");
			close_conn(pcb, hs);
			hs->flags &= ~TELNET_FLAG_CLOSE_CONNECTION;
		}

		// timeout and close connection if nothing has been received for N seconds
		hs->timeout++;
		if (hs->timeout > IDLE_TIMEOUT)
			close_conn(pcb,hs);
	}
	return ERR_OK;
}


/*------------------------------------------------------------------------------------------------*/
static err_t telnet_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
	struct telnet_state *hs;

	LWIP_UNUSED_ARG(len);

	hs = arg;

	if (hs->left > 0) {
		send_data(pcb, hs);
	} else {
		if (hs->flags & TELNET_FLAG_CLOSE_CONNECTION) {
			_CONSOLE( LogId, "closing connection\n");
			close_conn(pcb, hs);
			hs->flags &= ~TELNET_FLAG_CLOSE_CONNECTION;
		}
	}

	return ERR_OK;
}


/*------------------------------------------------------------------------------------------------*/
void sendopt(struct telnet_state *hs, u8_t option, u8_t value)
{
	hs->data_out[hs->left++] = TELNET_IAC;
	hs->data_out[hs->left++] = option;
	hs->data_out[hs->left++] = value;
	hs->data_out[hs->left] = 0;
}


/*------------------------------------------------------------------------------------------------*/
void get_char(struct tcp_pcb *pcb, struct telnet_state *hs, char c)
{
	int l;

	if (c == '\r') return;
	if (c != '\n')
	{
		l = strlen(hs->cmd_buffer);
		hs->cmd_buffer[l++] = c;
		hs->cmd_buffer[l] = 0;
	}
	else
	{
		cmd_parser(hs);		// handle command
		if (hs->left > 0)
		{
			send_data(pcb, hs);
			tcp_sent(pcb, telnet_sent);
		}
	}
}


/*------------------------------------------------------------------------------------------------*/
static err_t telnet_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	struct telnet_state *hs;
	char *q,c;
	int len;

	hs = arg;

	if (err == ERR_OK && p != NULL) {

		/* Inform TCP that we have taken the data. */
		tcp_recved(pcb, p->tot_len);
		hs->timeout = 0;	// reset idletime ount

		q = p->payload;
		len = p->tot_len;

		while(len > 0)
		{
			c = *q++;
			--len;

			switch (hs->state) {
				case STATE_IAC:
					if(c == TELNET_IAC) {
						hs->cmd_buffer[0] = 0;
						get_char(pcb,hs,c);
						hs->state = STATE_NORMAL;
					} else {
						switch(c) {
							case TELNET_WILL:
								hs->state = STATE_WILL;
								break;
							case TELNET_WONT:
								hs->state = STATE_WONT;
								break;
							case TELNET_DO:
								hs->state = STATE_DO;
								break;
							case TELNET_DONT:
								hs->state = STATE_DONT;
								break;
							default:
								hs->state = STATE_NORMAL;
								break;
						}
					}
					break;

				case STATE_WILL:
					/* Reply with a DONT */
					sendopt(hs,TELNET_DONT, c);
					hs->state = STATE_NORMAL;
					break;
				case STATE_WONT:
					/* Reply with a DONT */
					sendopt(hs,TELNET_DONT, c);
					hs->state = STATE_NORMAL;
					break;
				case STATE_DO:
					/* Reply with a WONT */
					sendopt(hs,TELNET_WONT, c);
					hs->state = STATE_NORMAL;
					break;
				case STATE_DONT:
					/* Reply with a WONT */
					sendopt(hs,TELNET_WONT, c);
					hs->state = STATE_NORMAL;
					break;
				case STATE_NORMAL:
					if(c == TELNET_IAC) {
						hs->state = STATE_IAC;
					} else {
						get_char(pcb,hs,c);
					}
					break;
			}
		}

		//! \todo should check max length here
//		strcat(hs->cmd_buffer,p->payload);

		pbuf_free(p);
/*
		if (strchr(hs->cmd_buffer,'\r'))
		{
			cmd_parser(hs);		// handle command

			send_data(pcb, hs);

			// Tell TCP that we wish be to informed of data that has been
			// successfully sent by a call to the http_sent() function.
			tcp_sent(pcb, telnet_sent);
		}
*/

	}

	if (err == ERR_OK && p == NULL)
	{
		_CONSOLE( LogId, "remote host closed connection\n");
		close_conn(pcb, hs);
	}

	return ERR_OK;
}


/*------------------------------------------------------------------------------------------------*/
static err_t telnet_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	struct telnet_state *hs;

	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);

	tcp_setprio(pcb, TCP_PRIO_MIN);

	/* Allocate memory for the structure that holds the state of the
	connection. */
	hs = (struct telnet_state *)mem_malloc(sizeof(struct telnet_state));

	if (hs == NULL) {
		_CONSOLE( LogId, "telnet_accept: Out of memory\n");
		return ERR_MEM;
	}

	_CONSOLE( LogId, "New connection from %d.%d.%d.%d:%d\n",
			(pcb->remote_ip.addr >> 0) & 0xff,
			(pcb->remote_ip.addr >> 8) & 0xff,
			(pcb->remote_ip.addr >> 16) & 0xff,
			(pcb->remote_ip.addr >> 24) & 0xff,
			(pcb->remote_port) & 0xff
	);

	/* Initialize the structure. */
	hs->data_out = NULL;
	hs->cmd_buffer[0] = 0;
	hs->left = 0;
	hs->timeout = 0;
	hs->flags = 0;
	hs->state = STATE_NORMAL;

	/* Tell TCP that this is the structure we wish to be passed for our
	callbacks. */
	tcp_arg(pcb, hs);

	/* Tell TCP that we wish to be informed of incoming data by a call
	to the http_recv() function. */
	tcp_recv(pcb, telnet_recv);

	tcp_err(pcb, conn_err);
	tcp_poll(pcb, telnet_poll, 4);

	usprintf(tstr,"--- %s (AL%s) ---%s", Telnet_Welcom, VERSION_SW, Telnet_Prompt);
	hs->data_out = tstr;
	hs->left = strlen(tstr);
	send_data(pcb, hs);

	Telnet_NbActiveconnection++;

	return ERR_OK;
}


/*------------------------------------------------------------------------------------------------*/
void Telnet_Init(char* Prompt, char* Welcom)
{
	struct tcp_pcb *pcb;

	pcb = tcp_new();
	tcp_bind(pcb, IP_ADDR_ANY, 23);
	pcb = tcp_listen(pcb);
	tcp_accept(pcb, telnet_accept);

	Telnet_Prompt = Prompt;
	Telnet_Welcom = Welcom;
	Telnet_NbActiveconnection = 0;
}


/*------------------------------------------------------------------------------------------------*/

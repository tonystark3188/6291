#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


#include "camera.h"
#include "handle_request.h"
#include "debuglog.h"
#include "tcp_socket.h"
#include "gp_buffer.h"
#include "ap_error.h"
#include "malloc_args.h"
#include "parse_stream.h"
#include "do_handle_request.h"
#include "free_args.h"
#include "utils.h"

int handle_issue(struct my_buffer *req_buffer, struct my_buffer* resp_buffer)
{
	short ret = 0, ret2 = 0;
	short cmd;
	void* args = NULL;

	CK00 (ret = get_buffer_16(req_buffer,(unsigned short*)&cmd), err_no);
	if (cmd < 0 || cmd >= RCMD_COUNT)
		goto err_invalcmd;
	
	DEBG(1, "[receive cmd=%d]\n", cmd);
	CK00 (ret = handle_malloc_args[cmd](&args), err_handle);
	CK00 (ret = handle_request_buffer[cmd](req_buffer, args), err_handle);
	CK00 (ret = handle_request[cmd](resp_buffer, args), err_handle);
	handle_release_args[cmd](args);
	return ret;
	
err_handle:
	ret2 = ret;
	CK00 (ret = buffer_seek(resp_buffer, BUFSEEK_START, 0), err_args);
	CK00 (ret = put_buffer_16(resp_buffer, ret2), err_args);
	handle_release_args[cmd](args);
	return 0;
	
err_invalcmd:
	DEBG(1, "[invalid cmd=%d]\n", cmd);
	goto err_no;

err_args:
	handle_release_args[cmd](args);	
err_no:
	return ret;
	


}

int a_request_session(int sockfd)
{
	unsigned char* buf = NULL;
	int len = 0, ret = 0;

		
	CK00 (ret = tcp_recv(sockfd, &len, &buf), out);
	
	struct my_buffer req_buffer = {buf,len,len,0};
	struct my_buffer resp_buffer = {0};
	
	DEBG (1, "[log][print received buffer...]\n");
	print_buffer(buf, len);

	CK00 (ret = handle_issue(&req_buffer, &resp_buffer), err_reqbuf);
	
	DEBG (1, "[log][print respond buffer...]\n");
	print_buffer(resp_buffer.buf,resp_buffer.stream_len);
	
	CK00 (ret = tcp_send(sockfd, resp_buffer.buf, resp_buffer.stream_len), err_respbuf);
	
err_respbuf:
	free_buffer(&resp_buffer);
err_reqbuf:
	free_buffer(&req_buffer);
out:
	return ret;
}





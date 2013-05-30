
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include "iord.h"
#include "iord.fdh"

iord::iord()
{
	running = false;
	m_buffer_len = 0;
	m_buffer_cursor = 0;
	m_line_len = 0;
}

iord::~iord()
{
	if (running)
		stopprogram();
}

/*
void c------------------------------() {}
*/

static bool open_program(const char *program, char *args[], \
						 pid_t *pid_out, int *inpipe_out, int *outpipe_out)
{
int infd[2], outfd[2];

	if (pipe(infd) < 0 || pipe(outfd) < 0)
	{
		stat("open_program: failed to create pipes!");
		return 1;
	}
	
	pid_t child = vfork();
	if (child == 0)
	{	// this is the child
		// redirect stdout/stderr
		dup2(infd[1], STDOUT_FILENO);
		close(infd[0]);
		
		dup2(outfd[0], STDIN_FILENO);
		close(outfd[1]);
		
		// this makes spawned child a session group leader so
		// we can kill it and all of it's children at once.
		setpgid(0, 0);
		
		// execute the program
		execvp(program, args);
		
		fprintf(stdout, "** FAILED EXEC OF '%s'\n", program);
		fflush(stdout);
		exit(-1);
	}
	
	close(infd[1]);
	close(outfd[0]);
	
	if (inpipe_out)
		*inpipe_out = infd[0];
	if (outpipe_out)
		*outpipe_out = outfd[1];
	if (pid_out)
		*pid_out = child;
	
	stat("open_program: forked '%s' successfully, pid %d", program, child);
	return 0;
}

static void close_program(pid_t pid, int inpipe, int outpipe)
{
	kill(-pid, SIGTERM);
	usleep(50 * 1000);
	kill(-pid, SIGKILL);
	
	// prevent zombie processes
	int exitstat;
	waitpid(pid, &exitstat, 0);

	close(inpipe);
	close(outpipe);
}

/*
void c------------------------------() {}
*/

bool iord::startprogram(const char *program, const char *additional_args[])
{
	if (running)
		stopprogram();
	
	char *args[10000];
	args[0] = (char *)program;
	args[1] = NULL;
	if (additional_args)
	{
		for(int i=0;;i++)
		{
			args[i+1] = (char *)additional_args[i];
			if (additional_args[i]==NULL) break;
		}
	}
	
	if (open_program(program, args, &m_pid, &m_inpipe, &m_outpipe))
	{
		stat("iord::startprogram: popen failed: '%s'", program);
		return 1;
	}
	
	fcntl(m_inpipe, F_SETFL, fcntl(m_inpipe, F_GETFL, 0) | O_NONBLOCK);
	//fcntl(m_outpipe, F_SETFL, fcntl(m_outpipe, F_GETFL, 0) | O_NONBLOCK);
	
	m_buffer_len = 0;
	m_buffer_cursor = 0;
	m_line_len = 0;
	running = true;
	
	return 0;
}

bool iord::stopprogram()
{
	if (running)
	{
		close_program(m_pid, m_inpipe, m_outpipe);
		
		running = false;
		m_buffer_len = 0;
		m_buffer_cursor = 0;
		m_line_len = 0;
		return 0;
	}
	
	// not running!
	return 1;
}

/*
void c------------------------------() {}
*/

char *iord::readline()
{
	if (!running)
		return NULL;
	
	for(;;)
	{
		// process any data waiting in buffer and
		// copy it to m_line buffer.
		if (m_buffer_len > 0)
		{
			for(;m_buffer_cursor<m_buffer_len;m_buffer_cursor++)
			{
				char ch = m_buffer[m_buffer_cursor];
				switch(ch)
				{
					case 13: break;
					// received a whole line, return it
					case 10:
					{
						m_buffer_cursor++;
						m_line[m_line_len] = 0;
						m_line_len = 0;
						return strdup(m_line);
					}
					break;
					
					default:
						if (m_line_len < sizeof(m_line)-2)
							m_line[m_line_len++] = ch;
					break;
				}
			}
		}
		
		// if no data was waiting, try to read some in
		m_buffer_cursor = 0;
		m_buffer_len = 0;
		
		int nbytes = read(m_inpipe, m_buffer, sizeof(m_buffer)-1);
		
		if (nbytes == -1)
		{	// no data yet
			return NULL;
		}
		else if (nbytes == 0)
		{	// pipe closed
			stat("iord::readline: ALERT: PIPE CLOSED BY REMOTE PROCESS");
			return NULL;
		}
		else
		{	// new data read in
			m_buffer_len = nbytes;
		}
	}
}

void iord::sendline(const char *line)
{
	if (!running)
	{
		stat("sendline failed: program not running");
		return;
	}
	
	char line_buffer[10000];
	sprintf(line_buffer, "%s\r\n", line);
	
	write(m_outpipe, line_buffer, strlen(line_buffer));
}

/*
iord();
	~iord();
	
	void startprogram(const char *path);
	void stopprogram(void);
	
	const char *readline();
	void sendline(const char *line);
	
	bool running;
*/





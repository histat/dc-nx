
#ifndef _IORD_H
#define _IORD_H

#define MAX_LINE_BUFFER		32768

class iord
{
public:
	iord();
	~iord();
	
	bool startprogram(const char *path, const char *additional_args[]);
	bool stopprogram(void);
	
	char *readline();
	void sendline(const char *line);
	void clearline();
	
	bool running;
	
private:
	pid_t m_pid;
	int m_inpipe, m_outpipe;
	
	char m_buffer[MAX_LINE_BUFFER];
	int m_buffer_len;
	int m_buffer_cursor;
	
	char m_line[MAX_LINE_BUFFER];
	int m_line_len;
};

#endif

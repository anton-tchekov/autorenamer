#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>

static int test;

static int ends_with(const char *str, const char *suffix)
{
	if(!str || !suffix)
	{
		return 0;
	}

	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if(lensuffix >  lenstr)
	{
		return 0;
	}

	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

static char *detect_video_file(const char *str)
{
	static char *mp4 = ".mp4";
	static char *mkv = ".mkv";

	if(ends_with(str, mp4))
	{
		return mp4;
	}

	if(ends_with(str, mkv))
	{
		return mkv;
	}

	return NULL;
}

static int extract_episode_number(const char *str, int nth)
{
	int i = 0;
	static char epn[8];

	while(nth)
	{
		while(!isdigit(*str))
		{
			++str;
		}

		while(isdigit(*str))
		{
			++str;
		}

		--nth;
	}

	while(!isdigit(*str))
	{
		++str;
	}

	while(isdigit(*str))
	{
		epn[i++] = *str;
		++str;
	}

	epn[i] = '\0';
	return atoi(epn);
}

static char *filename_format(const char *in, int nth)
{
	static char out[4096];
	char *start = out;
	char *format = detect_video_file(in);

	if(format)
	{
		sprintf(start, "e%02d%s", extract_episode_number(in, nth), format);
	}
	else
	{
		int prev = 0;
		const char *s;
		char *p, c;
		for(s = in, p = out; (c = *s); ++s)
		{
			if(c >= 'A' && c <= 'Z')
			{
				c = tolower(c);
			}
			else if(c >= 'a' && c <= 'z')
			{
				/* Do nothing */
			}
			else if(c >= '0' && c <= '9')
			{
				/* Do nothing */
			}
			else if(c == '.' || c == '_' || c == '-')
			{
				/* Do nothing */
			}
			else if(c == ' ')
			{
				c = '_';
			}
			else
			{
				continue;
			}

			/* Skip spaces and underscores in a row */
			if(c == prev && (prev == '_' || prev == '-'))
			{
				continue;
			}

			prev = c;
			*p++ = c;
		}

		*p = '\0';

		/* Trim spaces in beginning and end */
		if(p > out)
		{
			if(*start == '_')
			{
				++start;
			}

			if(p[-1] == '_')
			{
				p[-1] = '\0';
			}
		}
	}

	return start;
}

static void handle_dir(char *path, char *new, int nth)
{
	int len;
	DIR *dp;
	struct dirent *ep;
	if(!(dp = opendir(path)))
	{
		return;
	}

	while((ep = readdir(dp)))
	{
		if(strcmp(ep->d_name, ".") == 0 ||
			strcmp(ep->d_name, "..") == 0)
		{
			continue;
		}

		len = strlen(path);
		path[len] = '/';
		strcpy(path + len + 1, ep->d_name);

		strcpy(new, path);
		strcpy(new + len + 1, filename_format(ep->d_name, nth));

		if(strcmp(path, new) != 0)
		{
			printf("Renaming %s to %s\n", path, new);
			if(!test)
			{
				if(rename(path, new))
				{
					printf("> RENAME FAILED\n");
				}
			}
		}

		if(ep->d_type == DT_DIR)
		{
			handle_dir(new, new, nth);
		}

		path[len] = '\0';
	}

	closedir(dp);
}

int main(int argc, char **argv)
{
	int nth = 0;
	int len;
	char path[2048], new[2048];
	if(argc < 2)
	{
		fprintf(stderr, "Usage: ./autorenamer path [nth-number] [test]\n"
				"AutoRenamer is a tool that recursively renames all files and directories to a specific format\n");
		return 0;
	}

	if(argc == 3)
	{
		if(strcmp(argv[2], "test") == 0)
		{
			test = 1;
		}
		else
		{
			nth = atoi(argv[2]);
		}
	}
	else if(argc == 4)
	{
		if(strcmp(argv[2], "test") == 0)
		{
			test = 1;
			nth = atoi(argv[3]);
		}
		else if(strcmp(argv[3], "test") == 0)
		{
			test = 1;
			nth = atoi(argv[2]);
		}
	}

	strcpy(path, argv[1]);
	len = strlen(path);
	if(path[len - 1] == '/')
	{
		path[len - 1] = '\0';
	}

	handle_dir(path, new, nth);
	return 0;
}

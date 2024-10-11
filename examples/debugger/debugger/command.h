#ifndef __COMMAND_H__
#define __COMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>

struct command_t
{
	struct list_head list;
	const char * name;
	const char * desc;

	void (*usage)(void);
	int (*exec)(int argc, char ** argv);
};

extern struct list_head __command_list;

struct command_t * search_command(const char * name);
int register_command(struct command_t * cmd);
int unregister_command(struct command_t * cmd);

#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_H__ */

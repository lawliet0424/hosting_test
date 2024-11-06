#ifndef FILE_DESCRIPTOR_H
#define FILE_DESCRIPTOR_H
#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/thread.h"
#include "filesys/file.h"

#define FILE_NAME_MAX 15
 // ------------------file descriptor struct----------------------
struct file_descriptor {
      // number of file descriptor
	int fd;
      // file pointer
	struct file *file;

	bool is_standard;

	//struct lock fd_lock;

	char name[FILE_NAME_MAX];
      // element of struct's linked list
    struct list_elem elem;
};

void init_fd_list(struct thread *t);
int assign_fd(struct thread *t);
void add_fd(struct thread *t, struct file_descriptor *fd);
struct file_descriptor *create_fd(struct file *file, int fd_num, const char *name);
struct file *find_pointing_file(struct thread *t, int fd_num);
bool delete_fd(struct thread *t, int fd_num);
struct file_descriptor *find_pointing_fd_element(struct thread *t, int fd_num);
struct file_descriptor *find_pointing_fd_element_by_name(struct thread *t, const char *name);

#endif

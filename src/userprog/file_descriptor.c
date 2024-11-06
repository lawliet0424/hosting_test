#include "file_descriptor.h"
#include "threads/thread.h"
#include "filesys/file.h"
#include <stdlib.h>

void init_fd_list(struct thread *t) {

	//--------FOR STDIN-------------
	struct file_descriptor *stdin_fd = malloc(sizeof(struct file_descriptor));
	if(stdin_fd != NULL) {
		stdin_fd -> fd = 0;
		stdin_fd -> file = NULL;
		stdin_fd -> is_standard = true;
		add_fd(t, stdin_fd);
	}

	//-------FOR STDOUT
	struct file_descriptor *stdout_fd = malloc(sizeof(struct file_descriptor));
	if(stdout_fd != NULL) {
		stdout_fd -> fd = 1;
		stdout_fd -> file = NULL;
		stdout_fd -> is_standard = true;
		add_fd(t, stdout_fd);
	}

	//------FOR STDERR
	struct file_descriptor *stderr_fd = malloc(sizeof(struct file_descriptor));
	if(stderr_fd != NULL) {
		stderr_fd -> fd = 2;
		stderr_fd -> file = NULL;
		stderr_fd -> is_standard = true;
		add_fd(t, stderr_fd);
	}
}

// ---------------------Find FD number for new file descriptor--------------------
int assign_fd(struct thread *t) {
	int fd = 3;

	if(list_empty(&t->file_descriptor) ) {
		init_fd_list(t);
	}
    
	struct list_elem *e;
    for(e = list_begin(&t -> file_descriptor); e != list_end(&t -> file_descriptor); e = list_next(e)) {
		struct file_descriptor *fd_element = list_entry(e, struct file_descriptor, elem);
		if(fd_element -> fd == fd) {
			fd ++;
		}
		else if(fd_element -> fd > fd) {
			return fd;
		}
	}
	return fd;
}

// --------------------Add FD into FD list in thread ---------------------------
void add_fd(struct thread *t, struct file_descriptor *fd) {
	list_push_back(&t -> file_descriptor, &fd -> elem);
}

// Create FD and set FD number and filename ------------------------------------
struct file_descriptor *create_fd(struct file *file, int fd_num, const char *file_name) {
	struct file_descriptor *fd = malloc(sizeof(struct file_descriptor));
	
	if(fd == NULL) {
		return NULL;
	}

	fd -> fd = fd_num;
	fd -> file = file;
	fd -> is_standard = false;
	
	if(file_name != NULL) {
		strlcpy(fd -> name,  file_name, FILE_NAME_MAX);
	}

	return fd;
}

struct file *find_pointing_file(struct thread *t, int fd_num) {
	struct list *fd = &t -> file_descriptor;

	if(fd == NULL) {
		return NULL;
	}

	struct list_elem *e;

	for(e = list_begin(fd); e != list_end(fd); e = list_next(e)) {
		struct file_descriptor *fd_element = list_entry(e, struct file_descriptor, elem);

		if(fd_element -> fd == fd_num) {
			return fd_element -> file;
		}
	}

	return NULL;
}

struct file_descriptor *find_pointing_fd_element(struct thread *t, int fd_num) {

    struct list *fd = &t -> file_descriptor;

	if(fd == NULL) {
		return NULL;
	}

    struct list_elem *e;

	for(e = list_begin(fd); e != list_end(fd); e = list_next(e)) {
		struct file_descriptor *fd_element = list_entry(e, struct file_descriptor, elem);
 
        if(fd_element -> fd == fd_num) {
            return fd_element;
        }
    }
 
    return NULL;
}

struct file_descriptor *find_pointing_fd_element_by_name(struct thread *t, const char *name) {
	struct list *fd = &t -> file_descriptor;

	if(fd == NULL) {
		return NULL;
	}

	struct list_elem *e;

	for(e = list_begin(fd); e != list_end(fd); e = list_next(e)) {
		struct file_descriptor *fd_element = list_entry(e, struct file_descriptor, elem);

		if(strcmp(fd_element -> name, name) == 0) {
			return fd_element;
		}
	}

	return NULL;
}

bool delete_fd(struct thread *t, int fd_num) {
	struct list_elem *e;
	
	if(fd_num == 0 || fd_num == 1 || fd_num == 2) {
		struct file_descriptor *fd_element = find_pointing_fd_element(t, fd_num);
		fd_element -> file = NULL;
		return true;
	}
	else {
		for(e = list_begin(&t -> file_descriptor); e != list_end(&t -> file_descriptor); e = list_next(e)) {
			struct file_descriptor *fd_element = list_entry(e, struct file_descriptor, elem);
	
			if(fd_element->fd == fd_num) {
				list_remove(&fd_element->elem);
				return true;
			}
		 }
	}	

	return false;
}

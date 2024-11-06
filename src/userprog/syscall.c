#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "lib/kernel/console.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "userprog/file_descriptor.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
 //   printf ("system call!\n");
	checkUserProvidedPointer(f -> esp);

	int syscallNum = *(int *) f -> esp;
	//printf("System call number: %d\n", syscallNum);

	switch (syscallNum) {
			case SYS_HALT:
				halt();
				break;
			case SYS_EXIT:
				checkUserProvidedPointer(f -> esp + 4);
				exit(*(int *)(f -> esp + 4));
				break;
			case SYS_EXEC:
				checkUserProvidedPointer(f -> esp + 4);
				f -> eax = exec(*(const char**)(f -> esp + 4));
				break;
			case SYS_WAIT:
				checkUserProvidedPointer(f -> esp + 4);
				f -> eax = wait(*(pid_t *)(f -> esp + 4));
				break;
			case SYS_CREATE:
				checkUserProvidedPointer(f -> esp + 4);
				checkUserProvidedPointer(f -> esp + 8);
				f -> eax = create(*(const char**)(f -> esp + 4), *(unsigned *)(f -> esp + 8));
				break;
			case SYS_REMOVE:
				checkUserProvidedPointer(f -> esp + 4);
				f -> eax = remove(*(const char**)(f -> esp + 4));
				break;
			case SYS_OPEN:
				checkUserProvidedPointer(f -> esp + 4);
				f -> eax = open(*(const char**)(f -> esp + 4));
				break;
			case SYS_FILESIZE:
				checkUserProvidedPointer(f -> esp + 4);
				f -> eax = filesize(*(int *)(f -> esp + 4));
				break;
			case SYS_READ:
				checkUserProvidedPointer(f -> esp + 4);
				checkUserProvidedPointer(f -> esp + 8);
				checkUserProvidedPointer(f -> esp + 12);
				f -> eax = read(*(int *)(f -> esp + 4), *(void **)(f -> esp + 8), *(unsigned *)(f -> esp + 12));
				break;
			case SYS_WRITE:
				checkUserProvidedPointer(f -> esp + 4);
				checkUserProvidedPointer(f -> esp + 8);
				checkUserProvidedPointer(f -> esp + 12);
				f -> eax = write(*(int *)(f -> esp + 4), *(const void **)(f -> esp + 8), *(unsigned *)(f -> esp + 12));
				break;
			case SYS_SEEK:
				checkUserProvidedPointer(f -> esp + 4);
				checkUserProvidedPointer(f -> esp + 8);
				seek(*(int *)(f -> esp + 4), *(unsigned *)(f -> esp + 8));
				break;
			case SYS_TELL:
				checkUserProvidedPointer(f -> esp + 4);
				f -> eax = tell(*(int *)(f -> esp + 4));
				break;
			case SYS_CLOSE:
				checkUserProvidedPointer(f -> esp + 4);
				close(*(int *)(f -> esp + 4));
				break;


	thread_exit ();
	}
}

void halt (void) {
	shutdown_power_off();
}

void exit (int status) {
//	printf ("start exit system call!\n");

	const char *currentThreadName = thread_name();
	struct thread *t = thread_current();

	printf ("%s: exit(%d)\n", currentThreadName, status);
	t -> exit_status = status;

	thread_exit();
}

pid_t exec (const char *file) {
		// ---------PARENT THREAD TID---------------
	//	file_deny_write(file);

//	printf ("start exec system call!\n");

	return process_execute(file);
}

int wait (pid_t pid) {
//	printf ("start wait system call!\n");
	return process_wait(pid);
}

bool create (const char *file, unsigned initial_size) {
	
	if(file == "") {
		exit(-1);
	}

	if(file ==  NULL) {
		exit(-1);
	}

	return filesys_create(file, initial_size);
}

bool remove (const char *file) {

		//check file------
//		struct file *f = filesys_open(file);
//		if( f == NULL) {
//			return false;
//		}

//		struct thread *t = thread_current();
//		struct file_descriptor *fd_element = find_pointing_fd_element_by_name(t, file);
//
//		if(fd_element != NULL) {
//			file_close(f);
//			return false;
//		}
//
//	bool isFileDeleted = filesys_remove(file);
		//file_close(f);

	//return isFileDeleted;
	return filesys_remove(file);
}

int open (const char *file) {
		// insert code
	if(file == "") {
		return -1;
	}

	if(file == NULL) {
		return -1;
	}
		// ----------------------
		// 1. find empty FD number
	struct thread *t = thread_current();
	int fd_num = assign_fd(t);
	//printf("Assigned file descriptor number: %d\n", fd_num);

	struct file *f = filesys_open(file);

	if(f == NULL) {
		//printf("Error: Failed to open file: %s\n", file);
		return -1;
	}

	struct file_descriptor *fd = create_fd(f, fd_num, file);

	if(fd == NULL) {
		//printf("Error: Failed to create file descriptor for file: %s\n", file);
		file_close(f);
		return -1;
	}
	
	add_fd(t, fd);
	// inster code
	//printf("Successfully opened file: %s with fd: %d\n", file, fd_num);

	//printf("Current file descriptors count: %d\n", list_size(&t->file_descriptor));
	return fd_num;

}


int filesize (int fd) {
	struct thread *t = thread_current();
	struct file *f = find_pointing_file(t, fd);

	if(f == NULL) {
		return -1;
	}

	return file_length(f);
}

int read (int fd, void *buffer, unsigned length) {

//	printf("start read system call!\n");
//	printf("fd num: %d", fd);

	int reading_length;
	struct thread *t = thread_current();
	struct file_descriptor *fd_element = find_pointing_fd_element(t, fd);

//	printf("bring fd element!\n");

		// check the read arrange is user address space----------------------
	if(fd_element == NULL) {
		return -1;
	}

	if(!is_user_vaddr(buffer + length - 1)) {
		exit(-1);
	}

	if(fd == 0 && fd_element -> is_standard) {
		for(reading_length = 0; reading_length < length; reading_length++) {
			int ch = input_getc();

			if(ch == '\0') {
				return -1;
			}

			((char *)buffer)[reading_length] = ch;
		}

		return reading_length;
	}

	if(fd == 1 || fd == 2) {
		return -1;
	}

	struct file *f = find_pointing_file(t, fd);
	
	if(f == NULL) {
		return -1;
	}

	unsigned file_size = file_length(f);

	if(file_size == 0) {
		return -1;
	}

	if(file_size < length) {
		length = file_size;
		return length;
	}

	reading_length = file_read(f, buffer, length);
	return reading_length;
}


	//    int reading;
//    for(reading = 0; reading < length; reading++) {
	//       int ch = input_getc();
	//
	//        if(ch == '\0') {
	//            return -1;
	//        }
	//
	//        ((char *)buffer)[reading] = ch;
	//    }


int write (int fd, const void *buffer, unsigned length) {
	
	//printf("start write system call!-----\n");
	//printf("fd num: %d\n", fd);

	int write_length;
    struct thread *t = thread_current();
	struct file_descriptor *fd_element = find_pointing_fd_element(t, fd);

		// 표준 출력인지 확인하기!!!!!! 아니라면 다른 것들과 동일하게 처리됨!!
	// Debugging output for fd_element
//    if (fd_element != NULL) {
//        printf("fd_element found: is_standard = %d\n", fd_element->is_standard);
//    } else {
//        printf("Error: fd_element is NULL for fd %d\n", fd);
//    }

	if(fd_element == NULL) {
		return -1;
	}
	
	if(fd == 1 && fd_element ->is_standard) {
		//printf("check file write!\n");
		putbuf(buffer, length);
		return length;
	}

	if(fd == 0 || fd == 2) {
		return -1;
	}

	struct file *f = find_pointing_file(t, fd);

	if(f == NULL) {
		return -1;
	}
	//printf("file write started!\n");
	write_length = file_write(f, buffer, length);
	return write_length;
}

void seek (int fd, unsigned position) {
	struct thread *t = thread_current();
	struct file *f = find_pointing_file(t, fd);

	file_seek(f, position);
}

unsigned tell (int fd) {
	struct thread *t = thread_current();
	struct file *f = find_pointing_file(t, fd);

	int offset = file_tell(f);

	return offset;
}

void close (int fd) {
	//printf ("start close system call!\n");
	struct thread *t = thread_current();
	struct file *f = find_pointing_file(t, fd);
	struct file_descriptor *fd_element = find_pointing_fd_element(t, fd);

	if (fd_element == NULL) {
		exit(-1);
	}
	
	bool isExistedFd = delete_fd(t, fd);

	if(isExistedFd) {
		if(f != NULL) {
			file_close(f);
		}

		if(fd < 3) {
			fd_element->is_standard = false;
			fd_element -> file = NULL;
		} else {
			free(fd_element);
		}
	} else {
		exit(-1);
	}
}

void checkUserProvidedPointer (void *vaddr) {
	if(vaddr == NULL) {
		exit(-1);
	}

	if(is_kernel_vaddr(vaddr)) {
		exit(-1);
	}

	if(!is_user_vaddr(vaddr)) {
		exit(-1);
	}

	struct thread *cur = thread_current();
	if (cur -> pagedir == NULL) {
		exit(-1);
	}

	if(pagedir_get_page(thread_current()->pagedir, vaddr) == NULL) {
		exit(-1);
    }

}

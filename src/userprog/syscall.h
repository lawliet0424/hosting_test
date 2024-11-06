#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>
//? Is it int? Not a tid_t?
typedef int pid_t;

void syscall_init (void);

void halt (void);
void exit (int status);
pid_t exec (const char *file);
int wait (pid_t pid);

// --------------- FILESYSTEM call ------------------------------
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);

int read (int fd, void *buffer, unsigned length);
int write (int fd, const void *buffer, unsigned length);

void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);
// -------------------------------------------------------------- 

void checkUserProvidedPointer (void *vaddr);

#endif /* userprog/syscall.h */

// 
// tsh - A tiny shell program with job control
// 
// <Put your name and login ID here>
// Thomas Payne, Devin Burke is my partner

using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>

#include "globals.h"
#include "jobs.h"
#include "helper-routines.h"

//
// Needed global variable definitions
//

static char prompt[] = "tsh> ";
int verbose = 0;

//
// You need to implement the functions eval, builtin_cmd, do_bgfg,
// waitfg, sigchld_handler, sigstp_handler, sigint_handler
//
// The code below provides the "prototypes" for those functions
// so that earlier code can refer to them. You need to fill in the
// function bodies below.
// 

void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

//
// main - The shell's main routine 
//
int main(int argc, char **argv) 
{
  int emit_prompt = 1; // emit prompt (default)

  //
  // Redirect stderr to stdout (so that driver will get all output
  // on the pipe connected to stdout)
  //
  dup2(1, 2);

  /* Parse the command line */
  char c;
  while ((c = getopt(argc, argv, "hvp")) != EOF) {
    switch (c) {
    case 'h':             // print help message
      usage();
      break;
    case 'v':             // emit additional diagnostic info
      verbose = 1;
      break;
    case 'p':             // don't print a prompt
      emit_prompt = 0;  // handy for automatic testing
      break;
    default:
      usage();
    }
  }

  //
  // Install the signal handlers
  //

  //
  // These are the ones you will need to implement
  //
  Signal(SIGINT,  sigint_handler);   // ctrl-c
  Signal(SIGTSTP, sigtstp_handler);  // ctrl-z
  Signal(SIGCHLD, sigchld_handler);  // Terminated or stopped child

  //
  // This one provides a clean way to kill the shell
  //
  Signal(SIGQUIT, sigquit_handler); 

  //
  // Initialize the job list
  //
  initjobs(jobs);

  //
  // Execute the shell's read/eval loop
  //
  for(;;) {
    //
    // Read command line
    //
    if (emit_prompt) {
      printf("%s", prompt);
      fflush(stdout);
    }

    char cmdline[MAXLINE];

    if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)) {
      app_error("fgets error");
    }
    //
    // End of file? (did user type ctrl-d?)
    //
    if (feof(stdin)) {
      fflush(stdout);
      exit(0);
    }

    //
    // Evaluate command line
    //
    eval(cmdline);
    fflush(stdout);
    fflush(stdout);
  } 

  exit(0); //control never reaches here
}
  
/////////////////////////////////////////////////////////////////////////////
//
// eval - Evaluate the command line that the user has just typed in
// 
// If the user has requested a built-in command (quit, jobs, bg or fg)
// then execute it immediately. Otherwise, fork a child process and
// run the job in the context of the child. If the job is running in
// the foreground, wait for it to terminate and then return.  Note:
// each child process must have a unique process group ID so that our
// background children don't receive SIGINT (SIGTSTP) from the kernel
// when we type ctrl-c (ctrl-z) at the keyboard.
//
void eval(char *cmdline) 
{
  printf("eval \n");
  /* Parse command line */
  //
  // The 'argv' vector is filled in by the parseline
  // routine below. It provides the arguments needed
  // for the execve() routine, which you'll need to
  // use below to launch a process.
  //
  char *argv[MAXARGS];
  pid_t pid;
  sigset_t badChild;
  //
  // The 'bg' variable is TRUE if the job should run
  // in background mode or FALSE if it should run in FG
  //
  int bg = parseline(cmdline, argv); 
  if (argv[0] == NULL){  
    return;   /* ignore empty lines */
  }
  
  if(!builtin_cmd(argv)){ //if not a build in command, fork child
    printf("eval not builtin_cmd \n");
    //block children signals so they dont interrupt parent process
     //
    sigemptyset(&badChild);
    sigaddset(&badChild, SIGCHLD);
    sigprocmask(SIG_BLOCK, &badChild, 0);
      
    //handles all child stuff
    //successfully created child
    if((pid = fork()) == 0){
        printf("fork() -> create child");
        // unblock signal when dealing with the children
        sigprocmask(SIG_UNBLOCK, &badChild, 0);
        
        // put the child in a new process group whos group id is identical to the child pid
        setpgid(0,0); 
        
        // executes child 
        if(execve(argv[0], argv, environ) < 0){
            printf("%s: Command not found. \n", argv[0]);
            exit(0);
        }
    }
    
    // when buildin command -- execute immeditally
    // when not builtin --  add to job list to be executed 
    // handles all parent stuff
    if(!bg){
        printf("eval in fg \n");
        addjob(jobs, pid, FG, cmdline);
        sigprocmask(SIG_UNBLOCK, &badChild, 0);
        waitfg(pid);
    }
    else{
        printf("eval in bg \n");
        addjob(jobs, pid, BG, cmdline);
        sigprocmask(SIG_UNBLOCK, &badChild, 0);
        printf("%d %s", pid, cmdline);
    
    }
  }
  return;
}


/////////////////////////////////////////////////////////////////////////////
//
// builtin_cmd - If the user has typed a built-in command then execute
// it immediately. The command name would be in argv[0] and
// is a C string. We've cast this to a C++ string type to simplify
// string comparisons; however, the do_bgfg routine will need 
// to use the argv array as well to look for a job number.
//
int builtin_cmd(char **argv) 
{
  printf("buildin_cmd \n");
  string cmd(argv[0]);
  if(cmd == "quit"){
      printf("quit \n");
      exit(0);
  }
  if(cmd == "jobs"){
      listjobs(jobs);
      return 1;
  }
  if(cmd == "bg" || cmd == "fg"){
      do_bgfg(argv);
      return 1;
  }
  return 0;     /* not a builtin command */
}

/////////////////////////////////////////////////////////////////////////////
//
// do_bgfg - Execute the builtin bg and fg commands
//
void do_bgfg(char **argv) 
{
  struct job_t *jobp=NULL;
    
  /* Ignore command if no argument */
  if (argv[1] == NULL) {
    printf("%s command requires PID or %%jobid argument\n", argv[0]);
    return;
  }
    
  /* Parse the required PID or %JID arg */
  if (isdigit(argv[1][0])) {
    pid_t pid = atoi(argv[1]);
    if (!(jobp = getjobpid(jobs, pid))) {
      printf("(%d): No such process\n", pid);
      return;
    }
  }
  else if (argv[1][0] == '%') {
    int jid = atoi(&argv[1][1]);
    if (!(jobp = getjobjid(jobs, jid))) {
      printf("%s: No such job\n", argv[1]);
      return;
    }
  }	    
  else {
    printf("%s: argument must be a PID or %%jobid\n", argv[0]);
    return;
  }

  //
  // You need to complete rest. At this point,
  // the variable 'jobp' is the job pointer
  // for the job ID specified as an argument.
  //
  // Your actions will depend on the specified command
  // so we've converted argv[0] to a string (cmd) for
  // your benefit.
  //
  string cmd(argv[0]);
  if(cmd == "fg"){
      printf("do_bgfg in foreground \n");
      if(jobp -> state == ST){
          kill(-jobp -> pid, SIGCONT); // kill sends is a way to send signals to all of your pid's, doesnt actually kill them
      }
      jobp -> state = FG; // changes the state to forground
      waitfg(jobp -> pid); // wait untill the child is executed
      
  }
  else if(cmd == "bg"){
      printf("do_bgfg in background \n");
      kill(-jobp -> pid, SIGCONT);
      jobp -> state = BG;
      printf("[%d] (%d) %s", jobp -> jid, jobp -> pid, jobp -> cmdline);
  }
  else
  {
	unix_error("error within do_bgfg");
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// waitfg - Block until process pid is no longer the foreground process
//
void waitfg(pid_t pid)
{
  printf("waitfg \n");
  // sleep until your fg pid is not in the list of jobs anymore
  while(pid == fgpid(jobs)){
      sleep(0);
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// Signal handlers
//


/////////////////////////////////////////////////////////////////////////////
//
// sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
//     a child job terminates (becomes a zombie), or stops because it
//     received a SIGSTOP or SIGTSTP signal. The handler reaps all
//     available zombie children, but doesn't wait for any other
//     currently running children to terminate.  
//
void sigchld_handler(int sig) 
{
  printf("sigchld_handler \n");
  pid_t pid;
  int status = -1;
  
  // pg.781
  // return 0 if none of the cildren in the wait set are stopped or terminated
  // we need waitpid funciton to return value > 0 before we reap a child
  // positive pid when you terminate
  // https://linux.die.net/man/2/waitpid
  // loops through child group and reaps all children untill there are no more childrent to delete
  // and there are no more signals to evaluate
  while((pid = waitpid(-1, &status, WUNTRACED|WNOHANG)) > 0){
      // returns true if the child terminated normally
      if(WIFEXITED(status)){
          printf("Job [%d] (%d) deleted normally\n", pid2jid(pid), pid);
          deletejob(jobs, pid);
      }
      
      // returns true if the child process terminated because signal was not caught
      if(WIFSIGNALED(status)){
          printf("WIFSIGNALED \n");
          printf("Job [%d] (%d) terminated by %d \n", pid2jid(pid), pid, WTERMSIG(status));
          deletejob(jobs, pid);
      }
      
      // returns true if signal is stopped
      if(WIFSTOPPED(status)){
          printf("WIFSTOPPED \n");
          getjobpid(jobs, pid) -> state = ST;
          printf("Job [%d] (%d) stopped by %d \n", pid2jid(pid), pid, WSTOPSIG(status));
      }
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// sigint_handler - The kernel sends a SIGINT to the shell whenver the
//    user types ctrl-c at the keyboard.  Catch it and send it along
//    to the foreground job.  
//
void sigint_handler(int sig) 
{
  printf("signint_handler \n");
  pid_t sigJob = fgpid(jobs);
  // sigJob = pid of group
  // if != 0, fg tasks exist in the pid group
  // thus we need to SIGINT or kill all the tasks in that group
  if(sigJob != 0){
      kill(-sigJob, SIGINT);
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
//
// sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
//     the user types ctrl-z at the keyboard. Catch it and suspend the
//     foreground job by sending it a SIGTSTP.  
//
void sigtstp_handler(int sig) 
{
  printf("signstp_handler \n");
  pid_t sigJob = fgpid(jobs);
  if(sigJob != 0){
      kill(-sigJob, SIGTSTP);
  }
  return;
}

/*********************
 * End signal handlers
 *********************/





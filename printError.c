/** 
 * Swetha Sankar
 * CISC 450
 * Programming Assignment #2
 * printError.c: Creates simple function to print error messages & exit out of program
 * */
void error(char *errmsg){
	perror(errmsg);
	exit(0);
}

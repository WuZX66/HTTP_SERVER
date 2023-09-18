#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXLINE 1024

int main(int argc, char *argv[], char *envp[])
{
     char *buf, *p;
     char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
     int n1 = 0, n2 = 0;
     /* Extract the two arguments */
     if ((buf = getenv("QUERY_STRING")) != NULL)
     {
          p = strchr(buf, '&');
          *p = '\0';
          strcpy(arg1, buf);
          strcpy(arg2, p + 1);
          n1 = atoi(arg1);
          n2 = atoi(arg2);
     }

     /* Make the response body */
     sprintf(content, "QUERY_STRING=%s", buf);
     sprintf(content, "Welcome to add.com: ");
     sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
     sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>",
             content, n1, n2, n1 + n2);
     sprintf(content, "%sThanks for visiting!\r\n", content);

     /* Generate the HTTP response */
     printf("HTTP/1.0 200 OK\r\n");
     printf("Content-length: %d\r\n", (int)strlen(content));
     printf("Content-type: text/html\r\n\r\n");
     printf("%s", content);
     fflush(stdout);
     return 0;
}
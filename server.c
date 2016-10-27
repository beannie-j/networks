#include <netinet/in.h>    
#include <stdio.h>    
#include <stdlib.h>    
#include <sys/socket.h>    
#include <sys/stat.h>    
#include <sys/types.h>    
#include <unistd.h> 
#include <sys/time.h>  
#include <errno.h> 
#include <string.h>

#define PORT8088 8000

// http://localhost:8000/

  int create_socket, new_socket;    
  socklen_t addrlen;    
  int bufsize = 1024;    
  char *buffer;    // the clients request 

  long size;
  char* buf;

  struct sockaddr_in address;

  char http_not_found[] = "HTTP/1.0 404 Not Found\n";
  char http_ok[] = "HTTP/1.0 200 OK\n";

  char* bad_request_response = 
  "HTTP/1.0 400 Bad Request\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Bad Request</h1>\n"
  "  <p>This server did not understand your request.</p>\n"
  " </body>\n"
  "</html>\n";


  char* not_found_response_template = 

  "HTTP/1.0 404 Not Found\n"

  "Content-type: text/html\n"

  "\n"

  "<html>\n"

  " <body>\n"

  "  <h1>Not Found</h1>\n"

  "  <p>The requested URL %s was not found on this server.</p>\n"

  " </body>\n"

  "</html>\n";



  char* ok_response =

  "HTTP/1.0 200 OK\n"

  "Content-type: text/html\n"

  "\n";



  char* bad_method_response_template = 

  "HTTP/1.0 501 Method Not Implemented\n"

  "Content-type: text/html\n"

  "\n"

  "<html>\n"

  " <body>\n"

  "  <h1>Method Not Implemented</h1>\n"

  "  <p>The method %s is not implemented by this server.</p>\n"

  " </body>\n"

  "</html>\n";



  int bytesLeft;





void server_createAndBind() {



  if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0)

    {    

      printf("The socket was created\n");

    } 

    address.sin_family = AF_INET;    

    address.sin_addr.s_addr = INADDR_ANY;    

    address.sin_port = htons(PORT8088);    

    

   if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == 0)

    {    

      printf("Binding Socket\n");

    }   



}



void server_listen()

{

  if (listen(create_socket, 10) < 0) 

      {    

        perror("server: listen");    

        exit(1);    

      } 

}



void server_accept() 

{
  if ((new_socket = accept(create_socket, (struct sockaddr *) &address, &addrlen)) < 0) 
      {    
        perror("server: accept");    
        exit(1);    
      }    
      if(new_socket > 0)
      {    
        printf("1. The Client is connected...\n");
      }
}



const char* server_read() {
    memset(buffer, 0, sizeof(buffer));
    // optimise here
    int n = read(new_socket, buffer, 1024);

    if(n < 0) 
    {
      perror("Error ");
	    return (const char*)0;
    } 

    else if(n == 0) 

    {
      perror("Error : client disconnected");
      return (const char*)0;
    }

    char* result = (char*)malloc(n + 1);
    memcpy(result, buffer, n);
    result[n] = 0;
    // how to stop reading the buffer?
    return result;
}


void server_write() {
  write(new_socket, "HTTP/1.1 200 OK\n", 16);
  write(new_socket, "Content-length: 46\n", 19);
  write(new_socket, "Content-Type: text/html\n\n", 25);
  write(new_socket, "<html><body><H1>The requested file was found by the server</H1></body></html>",52);    
}

void server_write_filenotfound()
{
  write(new_socket, "HTTP/1.0 404 Not Found\n", 23);
  write(new_socket, "Content-length: 46\n", 19);
  write(new_socket, "Content-type: text/html\n", 24);
  write(new_socket, "<html><body><H1>The requested file was not found by the server</H1></body></html>", 82);
}

void server_write_ok() {
  char http_ok1[] = "HTTP/1.0 200 OK\n";
  char http_ok2[] = "Content-length: 46\n";
  char http_ok3[] = "Content-Type: text/html\n";
  char http_ok4[] = "<html><h1>File found</h1></html>\n\n";

  write(new_socket, "HTTP/1.0 200 OK\n", strlen("HTTP/1.0 200 OK\n"));
  write(new_socket, "Content-length: 46\n", strlen("Content-length: 46\n"));
  write(new_socket, "Content-Type: text/html\n", strlen("Content-Type: text/html\n"));
  write(new_socket, "<html><body><H1>The requested file was found</H1></body></html>",
    strlen("<html><body><H1>The requested file was found</H1></body></html>"));

}


int read_file(const char* path)
{
    printf("Reading file %s\n", path + 1);
    FILE* file = fopen(path + 1, "rb");
    if (file == NULL)
    {
        printf("File %s not found!", path + 1);
        // TODO: Send 404
        return 0;
    } else {
        printf("%s\n", http_ok);
        // server_write_ok();
    }
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buf = (char*)malloc(size + 1);
    buf[size] = 0;
    fread(buf, 1, size, file);
    fclose(file);

    printf("%s\n", buf);
    return 1;
}


    

int main() {   

  buf = (char*)malloc(size + 1);
  buffer = malloc(1024);  
  server_createAndBind();   

    while (1) 

    {    
      server_listen();
      server_accept();     

      const char* message = server_read();
      printf("message : ");
      printf("%s\n", message);
      const char* pos = strstr(message, "\n");
      int size = pos - message;
      char* get = (const char*)malloc(size + 1);
      memcpy(get, message, size);
      get[size] = 0;

      int tc = 0;
      char* token = strtok(get, " ");
      while (token)
      {
          token = strtok(NULL, " ");
          if (tc == 0)
          {
              if (read_file(token)) 
              {
                write(new_socket, ok_response, strlen(ok_response));
                write(new_socket, buf, strlen(buf));
              }  
              else
                  write(new_socket, not_found_response_template, strlen(not_found_response_template));   
          }
          tc++;
      }
      close(new_socket);    

    }    

  close(create_socket);    

  return 0;    

}

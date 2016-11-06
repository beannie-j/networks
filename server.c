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
#include <pthread.h>

#define PORT8088 8080

// http://localhost:8000/
// make sure to compile with 
// gcc server.c -o s -lpthread
// serve multiple documents in parallel

  void helper_read_file(int socket);
  void* connection_thread(void* socket_ptr);

  FILE* file;
  int g_ServerSocket; 
  int sock, *thread_socket;   
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
  "  <p>The requested URL was not found on this server.</p>\n"
  " </body>\n"
  "</html>\n";

  char* ok_response =
  "HTTP/1.0 200 OK\n"
  "Content-type: text/html\n"
  "\n"
  " <body>\n"
  "  <h1>File Found</h1>\n"
  "  <p>The requested URL was found on this server.</p>\n"
  " </body>\n"
  "</html>\n";

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
   if ((g_ServerSocket = socket(AF_INET, SOCK_STREAM, 0)) > 0)
   {    
      printf("The socket was created\n");
   } 
   address.sin_family = AF_INET;    
   address.sin_addr.s_addr = INADDR_ANY;    
   address.sin_port = htons(PORT8088);    

   if (bind(g_ServerSocket, (struct sockaddr *) &address, sizeof(address)) == 0)
   {    
      printf("Binding Socket\n");
   }   
}

void server_listen()
{
    if (listen(g_ServerSocket, 10) < 0) 
    {    
        perror("server: listen");    
        exit(1);    
    } 
}



void server_accept() 
{
  int socket = accept(g_ServerSocket, (struct sockaddr *) &address, &addrlen);
  if (socket < 0) 
  {    
    perror("server: accept");    
    exit(1);
  }   
  int* thread_socket = (int*)malloc(sizeof(int));
  *thread_socket = socket;

  printf("Starting connection thread...\n");
  pthread_t sniffer_thread;

  if(pthread_create(&sniffer_thread, NULL, connection_thread, thread_socket) < 0) {
    perror("Could not create thread ");
    exit(1);
  }
}

void* connection_thread(void* socket_ptr) {
  printf("1. The Client is connected...\n");

  int socket = *(int*)socket_ptr;
  helper_read_file(socket);
  printf("Closing client socket...\n");
  close(socket);

  return 0;
}





const char* server_read(int socket) {
    memset(buffer, 0, bufsize);
    // optimise here
    int n = read(socket, buffer, 1024);
    if(n < 0) 
    {
      perror("server_read error ");
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



void server_write(int socket) {
  write(socket, "HTTP/1.1 200 OK\n", 16);
  write(socket, "Content-length: 46\n", 19);
  write(socket, "Content-Type: text/html\n\n", 25);
  write(socket, "<html><body><H1>The requested file was found by the server</H1></body></html>",52);    

}



void server_write_filenotfound(int socket)
{
  write(socket, "HTTP/1.0 404 Not Found\n", 23);
  write(socket, "Content-length: 46\n", 19);
  write(socket, "Content-type: text/html\n", 24);
  write(socket, "<html><body><H1>The requested file was not found by the server</H1></body></html>", 82);
}

void server_write_ok(int socket) {
  char http_ok1[] = "HTTP/1.0 200 OK\n";
  char http_ok2[] = "Content-length: 46\n";
  char http_ok3[] = "Content-Type: text/html\n";
  char http_ok4[] = "<html><h1>File found</h1></html>\n\n";

  write(socket, "HTTP/1.0 200 OK\n", strlen("HTTP/1.0 200 OK\n"));
  write(socket, "Content-length: 46\n", strlen("Content-length: 46\n"));
  write(socket, "Content-Type: text/html\n", strlen("Content-Type: text/html\n"));
  write(socket, "<html><body><H1>The requested file was found</H1></body></html>",
  strlen("<html><body><H1>The requested file was found</H1></body></html>"));
}



int read_file(const char* path)
{
    printf("Reading file %s\n", path + 1);
    file = fopen(path + 1, "rb");
    if (file == NULL)
    {
        printf("File %s not found!\n", path + 1);
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


void helper_read_file(int socket) 
{
    const char* message = server_read(socket);
    printf("message : ");
    printf("%s\n", message);
    const char* pos = strstr(message, "\n");
    int size = pos - message;
    char* get = malloc(size + 1);
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
              write(socket, ok_response, strlen(ok_response));
              // Prints message in file
              write(socket, buf, strlen(buf));
            }  
            else
                write(socket, not_found_response_template, strlen(not_found_response_template));   
        }
        tc++;
    }
}

int main() {   
  buf = (char*)malloc(size + 1);
  buffer = malloc(1024);  
  server_createAndBind();   
  server_listen();

  while (1) 
  {    
    server_accept(); 
    //helper_read_file();

  }    



  close(g_ServerSocket);    



  return 0;    



}


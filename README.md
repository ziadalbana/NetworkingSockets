# Networking_sockets
A Simple web client that communicates with a web server using a restricted subset of HTTP

 web server should accept incoming connection requests. It should then look for the GET request
and pick out the name of the requested file. If the request is POST then it sends an OK message and
waits for the uploaded file from the client. Note that a GET request from a real WWW client may have
several lines of optional information following the GET. These optional lines, though, will be terminated
by a blank line (i.e., a line containing zero or more spaces, terminated by a ‘\r\n’ (carriage return then
newline characters). Your server should first print out the received command as well as any optional
lines following it (and preceding the empty line). 
The server should then respond with the line, this is a very simple version of the real HTTP reply
message:
HTTP/1.1 200 OK\r\n
then in case of GET command only:
{data, data, ...., data}

#include <iostream>
#include <cassert>
#include <string>
#include <algorithm>

#include <netinet/in.h>
#include <uv.h>


std::string
random_string(size_t length) {
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

const std::string CONTENT_20K = random_string(20480);
const std::string RESP_20K = std::string{"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 20480\r\n\r\n"} + CONTENT_20K;

size_t handled_requests{0};


void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);


uv_loop_t loop;


void
on_send_complete(uv_write_t* write_req, int status)
{
    (void)status;
    if (++handled_requests >= 2) {
        uv_stop(&loop);
    };
    free(write_req);
}


void
send_response(uv_stream_t* client)
{
    std::string resp{"HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: 5\r\n\r\nhello"};
    resp = RESP_20K;
    uv_buf_t resp_buf;
    alloc_buffer(nullptr, resp.size(), &resp_buf);
    resp_buf.base = const_cast<char*>(resp.c_str());

    uv_write_t* write_req = static_cast<uv_write_t*>(malloc(sizeof(uv_write_t)));
    write_req->data = resp_buf.base;
    uv_write(write_req, client, &resp_buf, 1, on_send_complete);

    uv_close(reinterpret_cast<uv_handle_t*>(client), (uv_close_cb)free);
}


void
on_data(uv_stream_t* client, ssize_t buf_size, const uv_buf_t* buf)
{
    (void)buf;
    if (buf_size > 0) {
        // std::cout << std::string(buf->base, buf_size) << '\n';
    }
    send_response(client);
}


void
alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    (void)handle;
    buf->base = static_cast<char*>(malloc(suggested_size));
    buf->len = suggested_size;
}


void
on_new_connection(uv_stream_t* server, int status)
{
    assert(status == 0);

    uv_tcp_t* client = static_cast<uv_tcp_t*>(malloc(sizeof(uv_tcp_t)));
    uv_tcp_init(&loop, client);
    if (uv_accept(server, reinterpret_cast<uv_stream_t*>(client)) == 0) {
        uv_read_start(reinterpret_cast<uv_stream_t*>(client), alloc_buffer, on_data);
    }
    else {
      uv_close(reinterpret_cast<uv_handle_t*>(client), NULL);
    }
}


int
main(void)
{
    int ret = uv_loop_init(&loop);
    assert(ret == 0);

    uv_tcp_t server;
    uv_tcp_init(&loop, &server);
    struct sockaddr_in bind_addr;
    uv_ip4_addr("0.0.0.0", 8000, &bind_addr);
    uv_tcp_bind(&server, (const struct sockaddr*)&bind_addr, 0);

    uv_listen((uv_stream_t*) &server, 65535, on_new_connection);
    uv_run(&loop, UV_RUN_DEFAULT);

    uv_loop_close(&loop);

    return 0;
}

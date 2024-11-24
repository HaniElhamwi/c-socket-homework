#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PAYLOAD_SIZE 4096

struct per_session_data {
    unsigned char buffer[MAX_PAYLOAD_SIZE];
    size_t len;
};

static int callback_ws(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len) {
    struct per_session_data *psd = (struct per_session_data *)user;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("New connection established\n");
            break;

        case LWS_CALLBACK_RECEIVE:
            if (len < MAX_PAYLOAD_SIZE) {
                memcpy(psd->buffer, in, len);
                psd->len = len;
                printf("Received: %.*s\n", (int)len, (char *)in);

                // Broadcast the received message to all clients
                lws_broadcast(wsi->protocol, psd->buffer, psd->len);
            }
            break;

        case LWS_CALLBACK_CLOSED:
            printf("Connection closed\n");
            break;

        default:
            break;
    }

    return 0;
}

static const struct lws_protocols protocols[] = {
    {
        "example-protocol",   // Protocol name
        callback_ws,          // Protocol callback
        sizeof(struct per_session_data),  // Per session data size
        MAX_PAYLOAD_SIZE      // Maximum message size
    },
    { NULL, NULL, 0, 0 } // Terminator for protocols array
};

int main() {
    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof(info));
    info.port = 3001;
    info.protocols = protocols;
    info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "Failed to create WebSocket context\n");
        return -1;
    }

    printf("Server is running on port 3001\n");

    // Run the WebSocket server
    while (lws_service(context, 1000) >= 0);

    lws_context_destroy(context);
    return 0;
}

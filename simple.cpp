#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/URI.h"
#include "Poco/StreamCopier.h"


#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"

#include <iostream>
#include <fstream>
#include <string>

#include <unordered_map>

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPMessage;
using Poco::Net::WebSocket;

using Poco::JSON::Parser;

using Poco::URI;
using Poco::StreamCopier;
using Poco::Dynamic::Var;
using Poco::JSON::Array;
using Poco::JSON::Object;

typedef struct tmessage {
    std::string content;
    std::string to;
} message;

int main(int args,char **argv) {
    /*
    std::ofstream out("out.txt");
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());
    */
    URI uri("http://localhost:9222/json");
    std::string path(uri.getPathAndQuery());

    HTTPClientSession session(uri.getHost(), uri.getPort());
    HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    HTTPResponse response;
    // writeLog(MessageType::Debug, "Send request");

    session.sendRequest(request);
    std::unordered_map<std::string, message> sendedMsg;
    // writeLog(MessageType::Debug, "Recieve response");
    std::istream &rs = session.receiveResponse(response);
    std::string s(std::istreambuf_iterator<char>(rs), {});
    //   writeLog(MessageType::Debug, "Response result0 %s",  s);
    std::string currentUserName;
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
        Parser parser;

        Var result = parser.parse(s);
        //writeLog(MessageType::Debug, "Init parser");

        Array::Ptr arr = result.extract<Array::Ptr>();
        Object::Ptr object = arr->getObject(1);
        // writeLog(MessageType::Debug, "getObject0");
        // writeLog(MessageType::Debug, "Response result %s",  result.toString());

        std::string webSocketUrl = object->get("webSocketDebuggerUrl").toString();

        // writeLog(MessageType::General,"Websocket URL: %s",webSocketUrl);

        std::cout << "Websocket URL: " << webSocketUrl << "\n";
        URI wsURI(webSocketUrl);

        std::cout << wsURI.getPath() << " PATH\n";

        HTTPResponse response1;
        HTTPRequest request1(HTTPRequest::HTTP_GET, wsURI.getPath(), HTTPMessage::HTTP_1_1);
        HTTPClientSession session1(uri.getHost(), uri.getPort());
        WebSocket *socket = new WebSocket(session1, request1, response1);

        char const *enableNetwork = "{\"id\": 1, \"method\": \"Network.enable\"}";

        socket->sendFrame(enableNetwork, strlen(enableNetwork), WebSocket::FRAME_TEXT);

        //std::cout << "Len " << len << "\n";
        constexpr int bufSize = 131072;

        std::string receiveBuff(bufSize, '\0');
        Poco::Buffer<char> buffer(bufSize);
        for (;;) {
            int flags = 0;
            buffer.resize(0);
            int rlen = socket->receiveFrame(buffer, flags);

            std::string json(buffer.begin(), buffer.end());
            Var result = parser.parse(json);
            Object::Ptr object = result.extract<Object::Ptr>();
            if (object->has("id") && object->getValue<int>("id") == INT_MIN && object->has("result")) {
                auto resultMsg = object->getObject("result");
                std::string jsonBody = resultMsg->getValue<std::string>("body");
                if (jsonBody[0] == '{') {
                    Var info = parser.parse(jsonBody);
                    Object::Ptr body = info.extract<Object::Ptr>();
                    if (body->has("channel")) {
                        std::string channel = body->getValue<std::string>("channel");
                        auto message = body->getObject("message");
                        std::string text = message->getValue<std::string>("text");
                        std::string user = message->getValue<std::string>("user");
                        std::cout << "Message is sent \n" << "user :" << user << "\n";
                        std::cout << "channel : " << channel << "\n";
                        std::cout << "text : " << text << "\n";
                    }
                }
            }

            if (object->has("params")) {
                auto params = object->getObject("params");
                if (object->getValue<std::string>("method") == "Network.loadingFinished") {
                    //Вызов метода для получения тела респонса
                    auto requestId = params->getValue<std::string>("requestId");
                    //std::cout << "Requset id " << requestId << "\n";
                    int f = std::stof(requestId) * 1000;
                    std::string sendResponse = std::string("{\"id\":" + std::to_string(f) +
                                                           ", \"method\": \"Network.getResponseBody\", \"params\": {\"requestId\" : \"") +
                                               requestId + std::string("\"} }");
                    socket->sendFrame(sendResponse.c_str(), sendResponse.size(), WebSocket::FRAME_TEXT);
                }
            }
        }
    }
}
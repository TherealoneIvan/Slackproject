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
    //Если нужен вывод в файл
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

    session.sendRequest(request);
    std::unordered_map<std::string, message> sendedMsg;
    std::istream &rs = session.receiveResponse(response);
    std::string s(std::istreambuf_iterator<char>(rs), {});
    std::string currentUserName;

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
        Parser parser;
        Var result = parser.parse(s);
        Array::Ptr arr = result.extract<Array::Ptr>();
        Object::Ptr object = arr->getObject(1);
        std::string webSocketUrl = object->get("webSocketDebuggerUrl").toString();
        std::cout << "Websocket URL: " << webSocketUrl << "\n";
        URI wsURI(webSocketUrl);
        std::cout << wsURI.getPath() << " PATH\n";

        HTTPResponse response1;
        HTTPRequest request1(HTTPRequest::HTTP_GET, wsURI.getPath(), HTTPMessage::HTTP_1_1);
        HTTPClientSession session1(uri.getHost(), uri.getPort());
        WebSocket *socket = new WebSocket(session1, request1, response1);

        char const *enableNetwork = "{\"id\": 1, \"method\": \"Network.enable\"}";

        socket->sendFrame(enableNetwork, strlen(enableNetwork), WebSocket::FRAME_TEXT);

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
            //Определяем то, что сообщение кто-то отправил
            //TODO эта часть должна отвечать за сообщения в лс
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
                        std::cout << "text : " << text << "\n" << "Message was sent" << "\n";
                    }
                }
            }
            //Определяем то, что сообщение было отправлено в канал
            //TODO эта часть должна отвечать за паблик чаты
            if (object->has("method")) {
                if (object->getValue<std::string>("method") == "Network.webSocketFrameReceived") {
                    if (object->has("params")) {
                        auto params = object->getObject("params");
                        auto timestamp = params->getValue<std::string>("timestamp");
                        if (params->has("response")) {
                            auto response = params->getObject("response");
                            if (response->has("payloadData")) {
                                Var payloadResult = parser.parse(response->getValue<std::string>("payloadData"));
                                auto payloadData = payloadResult.extract<Object::Ptr>();
                                if (payloadData->has("channel")&&payloadData->has("client_msg_id")) {
                                    auto message = payloadData->getValue<std::string>("text");
                                    auto channel = payloadData->getValue<std::string>("channel");
                                    auto user = payloadData->getValue<std::string>("user");
                                    std::cout << "message is sent to public chat" << "\n";
                                    std::cout<<"message "+message<<std::endl;
                                    std::cout<<"user "+user<<std::endl;
                                    std::cout<<"channel "+channel<<std::endl;
                                    std::cout<<"timestamp "+timestamp<<std::endl;
                                    std::cout << "message was sent to public chat" << "\n";
                                }
                            }
                        }
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
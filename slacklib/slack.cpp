#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/StreamCopier.h"

#include "circularBuffer/circularBuffer.h"

#include "Poco/JSON/Array.h"

#include "slack.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>

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

void SlackInterceptor::fillInfo(std::string & workplace, std::string & name, std::string & addresse, std::string & body,
                                std::string & timestamp, bool wasSentBySessionOwner, std::string & title, bool fileWasSent) {
    message info;
    info.workplaceName = workplace;
    info.user = name;
    info.to = addresse;
    info.text = body;
    info.time = timestamp;
    info.isSender = true;
    info.hasFile = fileWasSent;
    if (fileWasSent) {
        info.fileName = title;
    }
    storage.put(info);
}

void SlackInterceptor::printInfo() {
    message info = storage.get();
    //std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "Message was sent:\n";
    std::cout << "Workplace : " << info.workplaceName << "\n";
    std::cout << "User : " << info.user << "\n";
    std::cout << "To : " << info.to << "\n";
    std::cout << "Text : " << info.text << "\n";
    std::cout << "Time : " << info.time << "\n";
    std::cout << (info.hasFile ? "File is attached : " + info.fileName : "No files are attached") << "\n";

    //TODO
    std::cout << (info.isSender ? "Was sent by user" : "Was accepted by user") << "\n";
}

void SlackInterceptor::createListeners() {
    //If output in file is needed
    /*
    std::ofstream out("out.txt");
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());
    */
    try {
        URI uri(address);

        std::string path(uri.getPathAndQuery());

        HTTPClientSession session(uri.getHost(), uri.getPort());
        HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
        HTTPResponse response;

        try {
            session.sendRequest(request);
        }
        catch (std::exception & e) {
            std::cerr << e.what() << std::endl;
        }
        //std::unordered_map<std::string, message> sendedMsg;
        std::istream &rs = session.receiveResponse(response);
        std::string s(std::istreambuf_iterator<char>(rs), {});
        std::string currentUserName;

        if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED) {
            Var result = parser.parse(s);
            Array::Ptr arr = result.extract<Array::Ptr>();
            //Object::Ptr object;
            for (size_t i = 0; i < arr->size(); ++i) {
                Object::Ptr object = arr->getObject(i);
                if (object->get("title").toString() != "index.jade")
                    try {
                        listeners.emplace_back(&SlackInterceptor::listenAndCatch, this, object, uri, i);
                    }
                    catch (std::exception &e) {
                        std::cerr << e.what() << std::endl;
                    }
                //std::thread thr(&SlackInterceptor::listenAndCatch, this, object, uri);
            }
            try {
                readerVec.emplace_back(&SlackInterceptor::storageService, this);
                //reader.join();
            }
            catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
            /*for (size_t i = 0; i < arr->size(); ++i) {
                listeners[i].join();
            }*/

        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void SlackInterceptor::storageService() {
    while(!stopRequested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (!storage.empty())
            printInfo();
    }
}

void SlackInterceptor::listenAndCatch(Poco::JSON::Object::Ptr object, Poco::URI uri, int index) {
    std::string workplace = object->get("title").toString();
    std::string webSocketUrl = object->get("webSocketDebuggerUrl").toString();
    std::cout << "Websocket URL: " << webSocketUrl << "\n";
    URI wsURI(webSocketUrl);
    std::cout << wsURI.getPath() << " PATH\n";

    HTTPResponse response1;
    HTTPRequest request1(HTTPRequest::HTTP_GET, wsURI.getPath(), HTTPMessage::HTTP_1_1);
    HTTPClientSession session1(uri.getHost(), uri.getPort());
    bool isConnected = false;
    while (!isConnected) {
        try {
            //WebSocket *socket = new WebSocket(session1, request1, response1);
            std::unique_ptr <WebSocket> socket = std::make_unique<WebSocket>(session1, request1, response1);
            isConnected = true;
            /*}
            catch (std::exception & e) {
                std::cout << e.what() << std::endl;
            }*/
            char const *enableNetwork = "{\"id\": 1, \"method\": \"Network.enable\"}";
            try {
                socket->sendFrame(enableNetwork, strlen(enableNetwork), WebSocket::FRAME_TEXT);
            }
            catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
            constexpr int bufSize = 131072;
            std::string receiveBuff(bufSize, '\0');
            Poco::Buffer<char> buffer(bufSize);

            while (!stopRequested) {
                int flags = 0;
                buffer.resize(0);
                try {
                    int rlen = socket->receiveFrame(buffer, flags);
                }
                catch (std::exception &e) {
                    std::cerr << e.what() << std::endl;
                }
                std::string json(buffer.begin(), buffer.end());
                Var result = parser.parse(json);
                Object::Ptr object = result.extract<Object::Ptr>();
                bool fileWasSent = false;
                //std::cout << result.toString() << "-------JSON \n";
                //Catch needed message and gather info
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
                                    if (payloadData->has("channel") && payloadData->has("client_msg_id")) {
                                        auto message = payloadData->getValue<std::string>("text");
                                        std::string title = "";
                                        if (payloadData->has("files")) {
                                            auto fileData = parser.parse(payloadData->getValue<std::string>("files"));
                                            Array::Ptr fileInfo = fileData.extract<Array::Ptr>();
                                            Object::Ptr fileAttrs = fileInfo->getObject(0);
                                            title = fileAttrs->getValue<std::string>("title");
                                            fileWasSent = true;
                                        }
                                        auto channel = payloadData->getValue<std::string>("channel");
                                        auto user = payloadData->getValue<std::string>("user");
                                        fillInfo(workplace, user, channel, message, timestamp, true, title, fileWasSent);
                                        //printInfo();
                                    }
                                }
                            }
                        }
                    }
                    if (object->getValue<std::string>("method") == "Network.loadingFinished") {
                        //Вызов метода для получения тела респонса
                        if (object->has("params")) {
                            auto params = object->getObject("params");
                            auto requestId = params->getValue<std::string>("requestId");
                            //std::cout << "Requset id " << requestId << "\n";
                            int f = std::stof(requestId) * 1000;
                            std::string sendResponse = "{\"id\":" + std::to_string(f) +
                                                                   ", \"method\": \"Network.getResponseBody\", \"params\": {\"requestId\" : \"" +
                                                       requestId + "\"} }";
                            socket->sendFrame(sendResponse.c_str(), sendResponse.size(), WebSocket::FRAME_TEXT);
                        }
                    }
                }
            }
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}


void SlackInterceptor::setAddress (std::string str) {
    address = std::move(str);
}

void SlackInterceptor::stop () {
    if (previousWasStart) {
        stopRequested = true;
        for (size_t i = 0; i < listeners.size(); ++i) {
            listeners[i].join();
        }
        try {
            readerVec[0].join();
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    } else {
        std::cerr << "Previous call was stop() \n";
    }
}

void SlackInterceptor::start() {
    if (!previousWasStart) {
        stopRequested = false;
        createListeners();
    } else {
        std::cerr << "Previous call was start() \n";
    }
}

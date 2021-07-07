/*!********************************************************************

 Audacium: A Digital Audio Editor

 @file NetworkManager.h
 @brief Declare a class for preforming HTTP requests.

 Dmitry Vedenko
 **********************************************************************/

#pragma once

#include <memory>
#include <string>

#include "NetworkManagerApi.h"

namespace audacium
{
namespace network_manager
{

class Request;
class IResponse;
class IResponseFactory;

using ResponsePtr = std::shared_ptr<IResponse>;

class NETWORK_MANAGER_API NetworkManager final
{
    NetworkManager ();
    ~NetworkManager ();
public:
    static NetworkManager& GetInstance();
    static void Terminate (); 

    ResponsePtr doGet (const Request& request);
    ResponsePtr doHead (const Request& request);
    ResponsePtr doDelete (const Request& request);
    ResponsePtr doPost (const Request& request, const void* data, size_t size);
    ResponsePtr doPut (const Request& request, const void* data, size_t size);

    void setProxy (const std::string& proxy);
private:
    std::unique_ptr<IResponseFactory> mResponseFactory;
};

}
}

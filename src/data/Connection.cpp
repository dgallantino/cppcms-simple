#include "Connection.h"

namespace database
{
    Connection* Connection::connection_ = nullptr;
    Connection *Connection::getInstance(const std::string& constr) {
        if (connection_ == nullptr) {
            connection_ = new Connection(constr);
        }
        return connection_;
    }
} // namespace database

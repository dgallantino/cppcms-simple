#ifndef __TOKENMANAGER_H__
#define __TOKENMANAGER_H__

#include <map>
#include <string>
#include <mutex>
#include <chrono>

struct TokenData {
    std::string data;  // JSON string data associated with the token
    std::chrono::system_clock::time_point expiration;
};

class TokenManager 
{
private:
    // Private constructor for singleton pattern
    TokenManager() = default;
    
    // Delete copy constructor and assignment operator
    TokenManager(const TokenManager&) = delete;
    TokenManager& operator=(const TokenManager&) = delete;
    
    // Map to store tokens with their data and expiration time
    std::map<std::string, TokenData> tokens_;
    
    // Mutex for thread safety
    mutable std::mutex tokens_mutex_;

public:
    // Static method to get the singleton instance
    static TokenManager& getInstance();
    
    // Add a new token with data and expiration time (default 1 hour)
    void addToken(const std::string& token, const std::string& data,
                  std::chrono::minutes duration = std::chrono::minutes(60));
    
    // Check if a token is valid (exists and not expired)
    bool isValidToken(const std::string& token) const;
    
    // Get token data if token is valid
    std::string getTokenData(const std::string& token) const;
    
    // Remove a specific token
    void removeToken(const std::string& token);
    
    // Remove all expired tokens
    void cleanupExpiredTokens();
    
    // Get all active tokens with their data (for debugging purposes)
    std::map<std::string, TokenData> getActiveTokens() const;
    
    // Clear all tokens
    void clearAllTokens();
    
    // Get token count
    size_t getTokenCount() const;
};

#endif // __TOKENMANAGER_H__
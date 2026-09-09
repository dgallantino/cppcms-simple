#include "TokenManager.h"
#include <algorithm>

// Static method to get the singleton instance
TokenManager& TokenManager::getInstance() 
{
    static TokenManager instance;
    return instance;
}

// Add a new token with data and expiration time
void TokenManager::addToken(const std::string& token, const std::string& data, std::chrono::minutes duration) 
{
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    
    auto expiration_time = std::chrono::system_clock::now() + duration;
    tokens_[token] = {data, expiration_time};
}

// Check if a token is valid (exists and not expired)
bool TokenManager::isValidToken(const std::string& token) const 
{
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    
    auto it = tokens_.find(token);
    if (it == tokens_.end()) {
        return false; // Token not found
    }
    
    // Check if token is expired
    auto now = std::chrono::system_clock::now();
    if (it->second.expiration < now) {
        return false; // Token expired
    }
    
    return true; // Token is valid
}

// Get token data if token is valid
std::string TokenManager::getTokenData(const std::string& token) const 
{
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    
    auto it = tokens_.find(token);
    if (it == tokens_.end()) {
        return ""; // Token not found
    }
    
    // Check if token is expired
    auto now = std::chrono::system_clock::now();
    if (it->second.expiration < now) {
        return ""; // Token expired
    }
    
    return it->second.data; // Return token data
}

// Remove a specific token
void TokenManager::removeToken(const std::string& token) 
{
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    tokens_.erase(token);
}

// Remove all expired tokens
void TokenManager::cleanupExpiredTokens() 
{
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    
    auto now = std::chrono::system_clock::now();
    
    // Use erase-remove idiom for map
    auto it = tokens_.begin();
    while (it != tokens_.end()) {
        if (it->second.expiration < now) {
            it = tokens_.erase(it);
        } else {
            ++it;
        }
    }
}

// Get all active tokens with their data (for debugging purposes)
std::map<std::string, TokenData> TokenManager::getActiveTokens() const 
{
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    
    std::map<std::string, TokenData> active_tokens;
    auto now = std::chrono::system_clock::now();
    
    for (const auto& pair : tokens_) {
        if (pair.second.expiration >= now) {
            active_tokens[pair.first] = pair.second;
        }
    }
    
    return active_tokens;
}

// Clear all tokens
void TokenManager::clearAllTokens() 
{
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    tokens_.clear();
}

// Get token count
size_t TokenManager::getTokenCount() const 
{
    std::lock_guard<std::mutex> lock(tokens_mutex_);
    return tokens_.size();
}
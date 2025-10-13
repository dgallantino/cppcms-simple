/*
 * Example usage of TokenManager with token and data storage
 * 
 * This example shows how to use the TokenManager singleton to:
 * 1. Store tokens with associated JSON data
 * 2. Validate tokens
 * 3. Retrieve token data
 * 4. Clean up expired tokens
 */

#include "helpers/TokenManager.h"
#include <iostream>
#include <string>

void demonstrateTokenManager() {
    // Get the singleton instance
    TokenManager& tokenManager = TokenManager::getInstance();
    
    // Example 1: Add a token with user data (JSON string)
    std::string userToken = "user_token_123";
    std::string userData = "{\"userId\":\"123\",\"username\":\"john_doe\",\"role\":\"user\",\"email\":\"john@example.com\"}";
    tokenManager.addToken(userToken, userData, std::chrono::minutes(30));
    
    // Example 2: Add an admin token with different data
    std::string adminToken = "admin_token_456";
    std::string adminData = "{\"userId\":\"456\",\"username\":\"admin\",\"role\":\"administrator\",\"permissions\":[\"read\",\"write\",\"delete\"]}";
    tokenManager.addToken(adminToken, adminData, std::chrono::hours(2));
    
    // Example 3: Add a temporary token
    std::string tempToken = "temp_token_789";
    std::string tempData = "{\"sessionId\":\"temp_session\",\"purpose\":\"password_reset\"}";
    tokenManager.addToken(tempToken, tempData, std::chrono::minutes(5));
    
    // Check if tokens are valid
    if (tokenManager.isValidToken(userToken)) {
        std::string data = tokenManager.getTokenData(userToken);
        std::cout << "User token is valid. Data: " << data << std::endl;
    }
    
    if (tokenManager.isValidToken(adminToken)) {
        std::string data = tokenManager.getTokenData(adminToken);
        std::cout << "Admin token is valid. Data: " << data << std::endl;
    }
    
    // Check token count
    std::cout << "Total tokens stored: " << tokenManager.getTokenCount() << std::endl;
    
    // Get all active tokens (for debugging)
    auto activeTokens = tokenManager.getActiveTokens();
    std::cout << "Active tokens:" << std::endl;
    for (const auto& tokenPair : activeTokens) {
        std::cout << "Token: " << tokenPair.first << ", Data: " << tokenPair.second.data << std::endl;
    }
    
    // Remove a specific token (e.g., on logout)
    tokenManager.removeToken(tempToken);
    std::cout << "Removed temp token. Remaining tokens: " << tokenManager.getTokenCount() << std::endl;
    
    // Clean up expired tokens
    tokenManager.cleanupExpiredTokens();
    std::cout << "After cleanup: " << tokenManager.getTokenCount() << " tokens remaining" << std::endl;
}

// Example integration with authentication flow
bool authenticateRequest(const std::string& token, std::string& userInfo) {
    TokenManager& tokenManager = TokenManager::getInstance();
    
    if (tokenManager.isValidToken(token)) {
        userInfo = tokenManager.getTokenData(token);
        return true;
    }
    
    return false;
}

void handleLogout(const std::string& token) {
    TokenManager& tokenManager = TokenManager::getInstance();
    tokenManager.removeToken(token);
}
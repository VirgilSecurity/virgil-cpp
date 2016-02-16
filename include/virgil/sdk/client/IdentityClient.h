/**
 * Copyright (C) 2015 Virgil Security Inc.
 *
 * Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3) Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef VIRGIL_SDK_IDENTITY_CLIENT_H
#define VIRGIL_SDK_IDENTITY_CLIENT_H

#include <virgil/sdk/client/IdentityClientBase.h>
#include <virgil/sdk/http/Request.h>
#include <virgil/sdk/http/Response.h>

namespace virgil {
namespace sdk {
    namespace client {
        /**
         * @brief Entrypoint for interacting with Identity Service PKI.
         */
        class IdentityClient final : public IdentityClientBase {
        public:
            IdentityClient(const std::string& accessToken, const std::string& baseServiceUri);

            virgil::sdk::model::VirgilCard getServiceVirgilCard() const override;

            void setServiceVirgilCard(const virgil::sdk::model::VirgilCard& identityServiceCard) override;

            std::string verify(const virgil::sdk::model::Identity& identity) override;

            virgil::sdk::model::ValidatedIdentity confirm(const std::string& actionId,
                                                          const std::string& confirmationCode,
                                                          const int timeToLive = 3600,
                                                          const int countToLive = 1) override;

            bool isValid(const virgil::sdk::model::ValidatedIdentity& validatedIdentity) override;

        private:
            std::string accessToken_;
            std::string baseServiceUri_;
            virgil::sdk::model::VirgilCard identityServiceCard_;

        private:
            void verifyResponse(const virgil::sdk::http::Response& response);
        };
    }
}
}

#endif /* VIRGIL_SDK_IDENTITY_CLIENT_H */

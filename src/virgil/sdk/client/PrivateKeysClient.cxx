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

#include <iostream>

#include <json.hpp>

#include <virgil/crypto/VirgilByteArray.h>
#include <virgil/crypto/VirgilCipher.h>
#include <virgil/crypto/foundation/VirgilBase64.h>

#include <virgil/sdk/Error.h>
#include <virgil/sdk/client/ClientConnection.h>
#include <virgil/sdk/client/PrivateKeysClient.h>
#include <virgil/sdk/client/VerifyResponse.h>
#include <virgil/sdk/endpoints/PrivateKeysEndpointUri.h>
#include <virgil/sdk/http/Request.h>
#include <virgil/sdk/http/Response.h>
#include <virgil/sdk/io/Marshaller.h>
#include <virgil/sdk/model/Identity.h>
#include <virgil/sdk/util/JsonKey.h>
#include <virgil/sdk/util/uuid.h>

using json = nlohmann::json;

using virgil::crypto::VirgilByteArray;
using virgil::crypto::VirgilCipher;
using virgil::crypto::foundation::VirgilBase64;

using virgil::sdk::Credentials;
using virgil::sdk::Error;
using virgil::sdk::client::ClientConnection;
using virgil::sdk::client::PrivateKeysClient;
using virgil::sdk::endpoints::PrivateKeysEndpointUri;
using virgil::sdk::http::Request;
using virgil::sdk::http::Response;
using virgil::sdk::io::Marshaller;
using virgil::sdk::model::ValidationToken;
using virgil::sdk::model::Identity;
using virgil::sdk::model::IdentityType;
using virgil::sdk::model::PrivateKey;
using virgil::sdk::model::VirgilCard;
using virgil::sdk::util::JsonKey;
using virgil::sdk::util::uuid;


PrivateKeysClient::PrivateKeysClient(const std::string& accessToken, const std::string& baseUri)
        : accessToken_(accessToken),
          baseServiceUri_(baseUri) {

}

VirgilCard PrivateKeysClient::getServiceVirgilCard() const {
    return privateKeysServiceCard_;
}

void PrivateKeysClient::setServiceVirgilCard(const VirgilCard& privateKeysServiceCard) {
    privateKeysServiceCard_ = privateKeysServiceCard;
}

void PrivateKeysClient::stash(const std::string& virgilCardId, const Credentials& credentials) {
    json payload = {
        { JsonKey::privateKey, VirgilBase64::encode( credentials.privateKey() ) },
        { JsonKey::virgilCardId, virgilCardId }
    };

    Request request = Request()
            .post()
            .baseAddress(baseServiceUri_)
            .endpoint( PrivateKeysEndpointUri::privateKeyStash() )
            .body( payload.dump() );

    ClientConnection connection(accessToken_);
    Request signRequest = connection.signRequest(virgilCardId, credentials, request);

    std::string encryptJsonBody = connection.encryptJsonBody(
            privateKeysServiceCard_,
            payload.dump()
    );

    signRequest.body(encryptJsonBody);
    Response response = connection.send(signRequest);
    connection.checkResponseError(response, Error::Action::PRIVATE_KEY_STASH);
}

PrivateKey PrivateKeysClient::get(const std::string& virgilCardId, const ValidationToken& validationToken) {
    Identity identity = validationToken.getIdentity();
    // Password to encrypt server response. Up to 31 characters
    std::string responsePassword = uuid();
    while(responsePassword.size() > 31) {
        responsePassword.pop_back();
    }

    json payload = {
        { JsonKey::identity, {
            { JsonKey::type, identity.getTypeAsString() },
            { JsonKey::value, identity.getValue() },
            { JsonKey::validationToken, validationToken.getToken() }
        }},
        { JsonKey::responsePassword, responsePassword },
        { JsonKey::virgilCardId, virgilCardId }
    };

    ClientConnection connection(accessToken_);
    std::string encryptedRequestJsonBody = connection.encryptJsonBody(
            privateKeysServiceCard_,
            payload.dump()
    );

    Request request = Request()
            .post()
            .baseAddress(baseServiceUri_)
            .endpoint( PrivateKeysEndpointUri::privateKeyGet() )
            .body(encryptedRequestJsonBody);

    Response response = connection.send(request);
    connection.checkResponseError(response, Error::Action::PRIVATE_KEY_GET);

    VirgilCipher cipher;
    VirgilByteArray decryptResponseBody = cipher.decryptWithPassword(
            VirgilBase64::decode( response.body() ),
            virgil::crypto::str2bytes(responsePassword)
    );

    PrivateKey privateKey = Marshaller<PrivateKey>::fromJson(
            virgil::crypto::bytes2str( decryptResponseBody )
    );

    return privateKey;
}

void PrivateKeysClient::destroy(const std::string& virgilCardId, const VirgilByteArray& publicKey,
        const Credentials& credentials) {
    json payload = {
        { JsonKey::virgilCardId, virgilCardId }
    };

    Request request = Request()
            .post()
            .baseAddress(baseServiceUri_)
            .endpoint( PrivateKeysEndpointUri::privateKeyDestroy() )
            .body(payload.dump());

    ClientConnection connection(accessToken_);
    Request signRequest = connection.signRequest(virgilCardId, credentials, request);
    std::string encryptJsonBody = connection.encryptJsonBody(privateKeysServiceCard_, signRequest.body());
    signRequest.body(encryptJsonBody);

    Response response = connection.send(signRequest);
    connection.checkResponseError(response, Error::Action::PRIVATE_KEY_DESTROY);
}
/**
 * @file test_with_mock_sdms.cpp UT for Handlers module.
 *
 * Copyright (C) Metaswitch Networks 2017
 * If license terms are provided to you in a COPYING file in the root directory
 * of the source code repository by which you are accessing this code, then
 * the license outlined in that COPYING file applies to your use.
 * Otherwise no rights are granted except for those provided to you by
 * Metaswitch Networks in a separate written agreement.
 */

#ifndef TESTWITHMOCKSDMS_H__
#define TESTWITHMOCKSDMS_H__

#include "test_utils.hpp"
#include <curl/curl.h>

#include "mockhttpstack.hpp"
#include "handlers.h"
#include "chronoshandlers.h"
#include "gtest/gtest.h"
#include "basetest.hpp"
#include "siptest.hpp"
#include "localstore.h"
#include "fakehssconnection.hpp"
#include "fakechronosconnection.hpp"
#include "test_interposer.hpp"
#include "mock_subscriber_data_manager.h"
#include "mock_impi_store.h"
#include "mock_hss_connection.h"
#include "rapidjson/document.h"

using namespace std;
using ::testing::_;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::SetArgPointee;
using ::testing::SaveArg;
using ::testing::SaveArgPointee;
using ::testing::InSequence;
using ::testing::ByRef;
using ::testing::NiceMock;

class TestWithMockSdms : public SipTest
{
  MockSubscriberDataManager* store;
  MockSubscriberDataManager* remote_store1;
  MockSubscriberDataManager* remote_store2;
  MockHttpStack* stack;
  MockHSSConnection* mock_hss;

  virtual void SetUp()
  {
    store = new MockSubscriberDataManager();
    remote_store1 = new MockSubscriberDataManager();
    remote_store2 = new MockSubscriberDataManager();
    mock_hss = new MockHSSConnection();
    stack = new MockHttpStack();
  }

  virtual void TearDown()
  {
    delete stack;
    delete remote_store1; remote_store1 = NULL;
    delete remote_store2; remote_store2 = NULL;
    delete store; store = NULL;
    delete mock_hss;
  }

  SubscriberDataManager::AoRPair* build_aor(std::string aor_id,
                                            bool include_subscription = true)
  {
    SubscriberDataManager::AoR* aor = new SubscriberDataManager::AoR(aor_id);
    int now = time(NULL);
    build_binding(aor, now);
    if (include_subscription)
    {
      build_subscription(aor, now);
    }
    aor->_scscf_uri = "sip:scscf.sprout.homedomain:5058;transport=TCP";
    SubscriberDataManager::AoR* aor2 = new SubscriberDataManager::AoR(*aor);
    SubscriberDataManager::AoRPair* aor_pair = new SubscriberDataManager::AoRPair(aor, aor2);

    return aor_pair;
  }

  SubscriberDataManager::AoR::Binding*
    build_binding(SubscriberDataManager::AoR* aor,
                  int now,
                  const std::string& id = "<urn:uuid:00000000-0000-0000-0000-b4dd32817622>:1")
  {
    SubscriberDataManager::AoR::Binding* b = aor->get_binding(std::string(id));
    b->_uri = std::string("<sip:6505550231@192.91.191.29:59934;transport=tcp;ob>");
    b->_cid = std::string("gfYHoZGaFaRNxhlV0WIwoS-f91NoJ2gq");
    b->_cseq = 17038;
    b->_expires = now + 5;
    b->_priority = 0;
    b->_path_headers.push_back(std::string("<sip:abcdefgh@bono-1.cw-ngv.com;lr>"));
    b->_params["+sip.instance"] = "\"<urn:uuid:00000000-0000-0000-0000-b4dd32817622>\"";
    b->_params["reg-id"] = "1";
    b->_params["+sip.ice"] = "";
    b->_emergency_registration = false;
    b->_private_id = "6505550231";
    return b;
  }

  SubscriberDataManager::AoR::Subscription*
    build_subscription(SubscriberDataManager::AoR* aor,
                       int now,
                       const std::string& id = "1234")
  {
    SubscriberDataManager::AoR::Subscription* s = aor->get_subscription(id);
    s->_req_uri = std::string("sip:5102175698@192.91.191.29:59934;transport=tcp");
    s->_from_uri = std::string("<sip:5102175698@cw-ngv.com>");
    s->_from_tag = std::string("4321");
    s->_to_uri = std::string("<sip:5102175698@cw-ngv.com>");
    s->_to_tag = std::string("1234");
    s->_cid = std::string("xyzabc@192.91.191.29");
    s->_route_uris.push_back(std::string("<sip:abcdefgh@bono-1.cw-ngv.com;lr>"));
    s->_expires = now + 300;
    return s;
  }
};

#endif

/**
  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#include <iostream>
#include <atscppapi/GlobalPlugin.h>
#include <atscppapi/TransactionPlugin.h>
#include <atscppapi/TransformationPlugin.h>
#include <atscppapi/PluginInit.h>
#include <atscppapi/Logger.h>

using namespace atscppapi;
using std::string;

namespace {
#define TAG "null_transformation"
}

class NullTransformationPlugin : public TransformationPlugin {
public:
  NullTransformationPlugin(Transaction &transaction, TransformationPlugin::Type xformType)
    : TransformationPlugin(transaction, xformType) {
    registerHook((xformType == TransformationPlugin::REQUEST_TRANSFORMATION) ? 
                 HOOK_SEND_REQUEST_HEADERS : HOOK_SEND_RESPONSE_HEADERS);
  }

  void handleSendRequestHeaders(Transaction &transaction) {
    transaction.getServerRequest().getHeaders()["X-Content-Transformed"] = "1";
    transaction.resume();
  }

  void handleSendResponseHeaders(Transaction &transaction) {
    transaction.getClientResponse().getHeaders()["X-Content-Transformed"] = "1";
    transaction.resume();
  }

  void consume(const string &data) {
    produce(data);
  }

  void handleInputComplete() {
    setOutputComplete();
  }

  virtual ~NullTransformationPlugin() {

  }

private:
};

class GlobalHookPlugin : public GlobalPlugin {
public:
  GlobalHookPlugin() {
    registerHook(HOOK_READ_REQUEST_HEADERS_POST_REMAP);
    registerHook(HOOK_READ_RESPONSE_HEADERS);
  }

  virtual void handleReadRequestHeadersPostRemap(Transaction &transaction) {
    transaction.addPlugin(new NullTransformationPlugin(transaction, TransformationPlugin::REQUEST_TRANSFORMATION));
    transaction.resume();
  }

  virtual void handleReadResponseHeaders(Transaction &transaction) {
    transaction.addPlugin(new NullTransformationPlugin(transaction, TransformationPlugin::RESPONSE_TRANSFORMATION));
    transaction.resume();
  }
};

void TSPluginInit(int argc ATSCPPAPI_UNUSED, const char *argv[] ATSCPPAPI_UNUSED) {
  TS_DEBUG(TAG, "TSPluginInit");
  new GlobalHookPlugin();
}

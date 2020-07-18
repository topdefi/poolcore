#pragma once

#include <tbb/concurrent_queue.h>
#include <string>
#include <thread>
#include <vector>
#include "rapidjson/document.h"

class CNetworkClientDispatcher;

struct CCoinInfo {
  enum EAddressType {
    EP2PKH = 1,
    EPS2H = 2,
    EBech32 = 4,
    EZAddr = 8
  };

  std::string Name;
  int64_t RationalPartSize;
  EAddressType PayoutAddressType;
  bool SegwitEnabled;

  std::vector<uint8_t> PubkeyAddressPrefix;
  std::vector<uint8_t> ScriptAddressPrefix;

  uint16_t DefaultRpcPort;

  bool checkAddress(const std::string &address, EAddressType type);
};

class CNetworkClient {
public:
  struct GetBalanceResult {
    int64_t Balance;
    int64_t Immatured;
  };

  struct SendMoneyResult {
    bool Success;
    std::string TxId;
    std::string Error;
    int64_t Remaining;
    int64_t Fee;
  };

  struct ListUnspentElement {
    std::string Address;
    int64_t Amount;
  };

  struct ListUnspentResult {
    std::vector<ListUnspentElement> Outs;
  };

  struct ZSendMoneyResult {
    std::string AsyncOperationId;
    std::string Error;
  };

public:
  ~CNetworkClient() {}

  virtual bool ioGetBalance(GetBalanceResult &result) = 0;
  virtual bool ioSendMoney(const char *address, int64_t value, SendMoneyResult &result) = 0;

  virtual void poll() = 0;

  void setDispatcher(CNetworkClientDispatcher *dispatcher) { Dispatcher_ = dispatcher; }

protected:
  CNetworkClientDispatcher *Dispatcher_ = nullptr;
};

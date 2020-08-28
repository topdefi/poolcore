#ifndef __BACKEND_DATA_H_
#define __BACKEND_DATA_H_

#include "poolcommon/uint256.h"
#include <list>
#include <string>
#include <vector>
#include <filesystem>
#include "p2putils/xmstream.h"

std::string partByHeight(uint64_t height);
std::string partByTime(time_t time);

typedef bool CheckAddressProcTy(const char*);

struct CShare {
  uint64_t UniqueShareId = 0;
  std::string userId;
  std::string workerId;
  int64_t height;
  int64_t value;
  double WorkValue;
  bool isBlock;
  std::string hash;
  int64_t generatedCoins;
  int64_t Time;
};

struct PoolFeeEntry {
  std::string User;
  float Percentage;
};

template<typename T>
class SelectorByWeight {
public:
  void add(const T &value, uint32_t weight) {
    Values.push_back(value);
    ValueIndexes.insert(ValueIndexes.end(), weight, Values.size()-1);
  }

  const T &get() const { return Values[ValueIndexes[rand() % ValueIndexes.size()]]; }

private:
  struct Entry {
    T Value;
    uint32_t Weight;
  };

private:
  std::vector<T> Values;
  std::vector<size_t> ValueIndexes;
};

struct PoolBackendConfig {
  bool isMaster;
  std::filesystem::path dbPath;
  std::chrono::seconds ShareLogFlushInterval = std::chrono::seconds(3);
  uint64_t ShareLogFileSizeLimit = 134217728;

  std::vector<PoolFeeEntry> PoolFee;
  unsigned RequiredConfirmations;
  int64_t DefaultPayoutThreshold;
  int64_t MinimalAllowedPayout;
  unsigned KeepRoundTime;
  unsigned KeepStatsTime;
  unsigned ConfirmationsCheckInterval;
  unsigned PayoutInterval;
  unsigned BalanceCheckInterval;
  std::chrono::minutes StatisticKeepTime = std::chrono::minutes(30);
  std::chrono::minutes StatisticWorkersPowerCalculateInterval = std::chrono::minutes(11);
  std::chrono::minutes StatisticPoolPowerCalculateInterval = std::chrono::minutes(5);
  std::chrono::minutes StatisticWorkersAggregateTime = std::chrono::minutes(5);
  std::chrono::minutes StatisticPoolAggregateTime = std::chrono::minutes(1);
  std::chrono::hours StatisticKeepWorkerNamesTime = std::chrono::hours(24);

  SelectorByWeight<std::string> MiningAddresses;
  std::string CoinBaseMsg;

  // ZEC specify
  std::string poolTAddr;
  std::string poolZAddr;
};

struct roundElement {
  std::string userId;
  int64_t shareValue;
};

struct payoutElement {
  enum { CurrentRecordVersion = 1 };  
  
  std::string Login;
  int64_t payoutValue;
  int64_t queued;
  std::string asyncOpId;
  
  payoutElement() {}
  payoutElement(const std::string &userIdArg, int64_t paymentValueArg, int64_t queuedArg) :
    Login(userIdArg), payoutValue(paymentValueArg), queued(queuedArg) {}
    
  bool deserializeValue(const void *data, size_t size);
  bool deserializeValue(xmstream &stream);  
  void serializeValue(xmstream &stream) const;    
};  

struct shareInfo {
  std::string type;
  int64_t count;
};

struct miningRound {
  enum { CurrentRecordVersion = 1 };
  
  uint64_t height;
  std::string blockHash;
  time_t time;    
    
  // aggregated share and payment value
  int64_t totalShareValue;
  int64_t availableCoins;
    
  std::list<roundElement> rounds;
  std::list<payoutElement> payouts;
    
  miningRound() {}
  miningRound(unsigned heightArg) : height(heightArg) {}
    
  friend bool operator<(const miningRound &L, const miningRound &R) { return L.height < R.height; }
    
  void clearQueued() {
    for (auto I = payouts.begin(), IE = payouts.end(); I != IE; ++I)
      I->queued = 0;
  }
    
  std::string getPartitionId() const { return "default"; }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;
  void dump();
};

struct UsersRecord {
  enum { CurrentRecordVersion = 1 };

  std::string Login;
  std::string EMail;
  std::string Name;
  std::string TwoFactorAuthData;
  uint256 PasswordHash;
  uint64_t RegistrationDate;
  bool IsActive;

  UsersRecord() {}
  std::string getPartitionId() const { return "default"; }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;
};

struct UserActionRecord {
  enum { CurrentRecordVersion = 1 };

  enum Type {
    UserActivate = 0,
    UserChangePassword,
    UserChangeEmail
  };

  uint512 Id;
  std::string Login;
  uint32_t Type;
  uint64_t CreationDate;

  UserActionRecord() {}
  std::string getPartitionId() const { return "default"; }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;
};

struct UserSessionRecord {
  enum { CurrentRecordVersion = 1 };

  uint512 Id;
  std::string Login;
  uint64_t LastAccessTime;
  bool Dirty = false;

  UserSessionRecord() {}
  std::string getPartitionId() const { return "default"; }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;

  void updateLastAccessTime(uint64_t time) {
    LastAccessTime = time;
    Dirty = true;
  }
};

struct UserSettingsRecord {
  enum { CurrentRecordVersion = 1 };

  std::string Login;
  std::string Coin;
  std::string Address;
  int64_t MinimalPayout;
  bool AutoPayout;

  UserSettingsRecord() {}
  std::string getPartitionId() const { return "default"; }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;
};

struct UserBalanceRecord {
  enum { CurrentRecordVersion = 1 };
  
  std::string Login;
  int64_t Balance;
  int64_t Requested;
  int64_t Paid;

  UserBalanceRecord() {}
  UserBalanceRecord(const std::string &userIdArg, int64_t) :
    Login(userIdArg), Balance(0), Requested(0), Paid(0) {}
      
  std::string getPartitionId() const { return "default"; }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;
};


struct FoundBlockRecord {
  enum { CurrentRecordVersion = 1 };
  
  uint64_t Height;
  std::string Hash;
  int64_t Time;
  int64_t AvailableCoins;
  std::string FoundBy;
  
  std::string getPartitionId() const { return partByHeight(Height); }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;
};

struct PoolBalanceRecord {
  enum { CurrentRecordVersion = 1 };
  
  int64_t Time;
  int64_t Balance;
  int64_t Immature;
  int64_t Users;
  int64_t Queued;
  int64_t Net;

  std::string getPartitionId() const { return partByTime(Time); }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;
};

struct StatsRecord {
  enum { CurrentRecordVersion = 1 };
  
  std::string Login;
  std::string WorkerId;
  int64_t Time;
  uint64_t ShareCount;
  double ShareWork;
  
  std::string getPartitionId() const { return partByTime(Time); }
  bool deserializeValue(xmstream &stream);
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;    
};

struct ShareStatsRecord {
  enum { CurrentRecordVersion = 1 };

  int64_t Time;
  int64_t Total;
  std::vector<shareInfo> Info;
  
  std::string getPartitionId() const { return partByTime(Time); }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;    
};

struct PayoutDbRecord {
  enum { CurrentRecordVersion = 1 };
  
  std::string userId;
  int64_t time;
  int64_t value;
  std::string transactionId;
  friend bool operator<(const PayoutDbRecord &r1, const PayoutDbRecord &r2) { return r1.userId < r2.userId; }
  
  std::string getPartitionId() const { return partByTime(time); }
  bool deserializeValue(const void *data, size_t size);
  void serializeKey(xmstream &stream) const;
  void serializeValue(xmstream &stream) const;      
};

#endif //__BACKEND_DATA_H_

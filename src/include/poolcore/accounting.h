#ifndef __ACCOUNTING_H_
#define __ACCOUNTING_H_

#include "backendData.h"
#include "usermgr.h"
#include "poolcommon/pool_generated.h"
#include "poolcommon/file.h"
#include "kvdb.h"
#include "poolcore/rocksdbBase.h"
#include <deque>
#include <list>
#include <map>
#include <string>

class p2pNode;
class p2pPeer;
class StatisticDb;

class AccountingDb {  
private:
  struct payoutAggregate {
    std::string userId;
    int64_t shareValue;
    int64_t payoutValue;
    payoutAggregate(const std::string& userIdArg, int64_t shareValueArg) :
      userId(userIdArg), shareValue(shareValueArg), payoutValue(0) {}
  };

private:
  const PoolBackendConfig &_cfg;
  CCoinInfo CoinInfo_;
  UserManager &UserManager_;
  p2pNode *_client;
  
  std::map<std::string, UserBalanceRecord> _balanceMap;
  std::map<std::string, int64_t> _currentScores;
  std::deque<miningRound*> _allRounds;
  std::set<miningRound*> _roundsWithPayouts;
  std::list<payoutElement> _payoutQueue;  
  
  FileDescriptor _sharesFd;
  FileDescriptor _payoutsFd;
  kvdb<rocksdbBase> _roundsDb;
  kvdb<rocksdbBase> _balanceDb;
  kvdb<rocksdbBase> _foundBlocksDb;
  kvdb<rocksdbBase> _poolBalanceDb;
  kvdb<rocksdbBase> _payoutDb;
  
  
public:
  AccountingDb(const PoolBackendConfig &config, const CCoinInfo &coinInfo, UserManager &userMgr);

  void updatePayoutFile();
  void cleanupRounds();
  
  void requestPayout(const std::string &address, int64_t value, bool force = false);
  void payoutSuccess(const std::string &address, int64_t value, int64_t fee, const std::string &transactionId);
  
  void addShare(const Share *share, const StatisticDb *statistic);
  void mergeRound(const Round *round);
  void checkBlockConfirmations();
  void makePayout();
  void checkBalance();
  
  std::list<payoutElement> &getPayoutsQueue() { return _payoutQueue; }
  kvdb<rocksdbBase> &getFoundBlocksDb() { return _foundBlocksDb; }
  kvdb<rocksdbBase> &getPoolBalanceDb() { return _poolBalanceDb; }
  kvdb<rocksdbBase> &getPayoutDb() { return _payoutDb; }
  kvdb<rocksdbBase> &getBalanceDb() { return _balanceDb; }
  
  void queryClientBalance(p2pPeer *peer, uint32_t id, const std::string &userId);
  void updateClientInfo(p2pPeer *, uint32_t, const std::string &, const std::string &, const std::string &, int64_t);
  
  void manualPayout(p2pPeer *peer,
                    uint32_t id,
                    const std::string &userId);  
  
  void resendBrokenTx(p2pPeer *peer,
                      uint32_t id,
                      const std::string &userId);
  
  void moveBalance(p2pPeer *peer, uint32_t id, const std::string &userId, const std::string &to);
};

#endif //__ACCOUNTING_H_

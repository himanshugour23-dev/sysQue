
#include <bits/stdc++.h>
using namespace std;

vector<string> process_transactions( vector<string> initialBalances, vector<string> requests, int maxTotalDebitPerAccount){
    int n = requests.size();
    map<string, int> acctHolderBalances;
    for (int i = 0; i < initialBalances.size(); i++) {
        stringstream ss(initialBalances[i]);
        string account, balance;
        getline(ss, account, ',');
        getline(ss, balance, ',');
        acctHolderBalances[account] = stoi(balance);
    }
    vector<string> ans;
    map<string, string> transactionRequests;
    map<string, string> transactionResults;
    for (int i = 0; i < n; i++) {
        string req = requests[i];
        stringstream ss(req);
        string type;
        getline(ss, type, ',');
        string transactionId;
        getline(ss, transactionId, ',');
        if (transactionRequests.count(transactionId)) {
            if (transactionRequests[transactionId] == req) {
                ans.push_back(transactionResults[transactionId]);
            }
            else {
                ans.push_back(
                    transactionId + ",IDEMPOTENCY_CONFLICT"
                );
            }
            continue;
        }
        transactionRequests[transactionId] = req;
        string result;
        if (type == "DEBIT" || type == "CREDIT") {
            string accountId, amount;
            getline(ss, accountId, ',');
            getline(ss, amount, ',');

            if (!acctHolderBalances.count(accountId)) {
                result = transactionId + ",DECLINED";
            }
            else {
                int amountInt = stoi(amount);
                int bal = acctHolderBalances[accountId];
                if (amountInt <= 0) {
                    result = transactionId + ",DECLINED";
                }
                else if (type == "DEBIT") {
                    if (bal >= amountInt) { acctHolderBalances[accountId] = bal - amountInt;
                        result = transactionId + ",APPROVED," +
                            accountId + "," +
                            to_string(acctHolderBalances[accountId]);
                    }
                    else {
                        result = transactionId + ",DECLINED";
                    }
                }
                else {
                    acctHolderBalances[accountId] = bal + amountInt;
                    result = transactionId + ",APPROVED," + accountId + "," + to_string(acctHolderBalances[accountId]);
                }
            }
        }
        else if (type == "TRANSFER") {
            string fromAccountId, toAccountId, amount;
            getline(ss, fromAccountId, ',');
            getline(ss, toAccountId, ',');
            getline(ss, amount, ',');
            int amountInt = stoi(amount);
            if (amountInt <= 0 || !acctHolderBalances.count(fromAccountId) ||!acctHolderBalances.count(toAccountId)) {
                result = transactionId + ",DECLINED";
            }
            else if (acctHolderBalances[fromAccountId] < amountInt) {
                result = transactionId + ",DECLINED";
            }
            else {
                acctHolderBalances[fromAccountId] -= amountInt;
                acctHolderBalances[toAccountId] += amountInt;
                result = transactionId + ",APPROVED," + fromAccountId + "," + to_string(acctHolderBalances[fromAccountId]) + "," + toAccountId + "," + to_string(acctHolderBalances[toAccountId]);
            }
        }
        transactionResults[transactionId] = result;
        ans.push_back(result);
    }
    return ans;
}

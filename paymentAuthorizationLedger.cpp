/*
Question 3: Payment Authorization Ledger

When a customer pays by card, Stripe works in two phases. First it authorizes the payment: the money isn’t taken yet, but a hold is placed on it so the customer can’t spend it elsewhere. Later it captures the payment, which actually moves the money. If the sale falls through, the hold is voided.
For this multi-part question, we will build a ledger that models this lifecycle. We won’t talk to any bank. You will implement a function process that returns a result for each incoming request. The implementation is broken down into five parts.
Terms used throughout
account: a customer’s account, identified by accountId. An account that has never been deposited to has balance 0.
balance: the money in the account.
hold: money reserved by an AUTH but still in the balance. Each hold belongs to one account and is identified by a unique authId.
held: the sum of the amounts of the account’s active holds.
available = balance - held. New holds are checked against this number.

Function Description
Complete the function process in the editor below.

Function Parameters

int holdSeconds: how long a hold lasts before it expires (used in Part 4).
string requests[n]: lines in the order received. Each line starts with a timestamp (a non-decreasing integer in seconds, ignored until Part 4), then an action. The format is defined by one of these schemas:
timestamp,DEPOSIT,accountId,amount: add money to an account.
timestamp,AUTH,authId,accountId,amount: try to place a hold on an account.
timestamp,CAPTURE,authId or timestamp,CAPTURE,authId,amount: collect money against a hold.
timestamp,VOID,authId: cancel a hold.
timestamp,REFUND,authId,amount: give money back (Part 5).
timestamp,BALANCE,accountId: query an account (Part 5).

Returns
string[n]: one result per request, in the same order. Each is OK, DECLINED, INVALID or EXPIRED, or, for BALANCE, the string balance,available.

Part 1: Deposits and authorizations (test cases 1-4)
DEPOSIT adds amount to the account’s balance and returns OK. AUTH creates a hold of amount if available >= amount and returns OK. Otherwise it returns DECLINED and no hold is created.

Example:

1,DEPOSIT,acc1,100 → OK
2,AUTH,a1,acc1,60 → OK (balance 100, held 60)
3,AUTH,a2,acc1,50 → DECLINED (available is 40)

Part 2: Capture and void (test cases 5-8)
CAPTURE,authId takes the full held amount out of the balance and ends the hold. It returns OK. 
VOID,authId ends the hold and releases the funds, with the balance unchanged. It returns OK. If authId is unknown,
or its hold was already captured or voided, return INVALID. A declined AUTH never became a hold, so its authId is unknown.

Example:

1,DEPOSIT,acc1,100 → OK
2,AUTH,a1,acc1,60 → OK
3,AUTH,a2,acc1,50 → DECLINED
4,CAPTURE,a1 → OK (balance 40, held 0)
5,AUTH,a2,acc1,40 → OK
6,VOID,a2 → OK
7,VOID,a2 → INVALID (already voided)
8,CAPTURE,a9 → INVALID (unknown)

Part 3: Partial capture (test cases 9-11)
CAPTURE,authId,amount captures only amount, which may be less than the hold (a customer ordered 3 items but only 2 shipped).
The captured amount leaves the balance and the entire hold ends, so the unused remainder is released immediately.
If amount is greater than the held amount, return INVALID and the hold stays active.
A capture is final: capturing the same auth again returns INVALID. CAPTURE without an amount still captures the full hold.

Example:

1,DEPOSIT,acc1,100 → OK
2,AUTH,a1,acc1,70 → OK
3,CAPTURE,a1,80 → INVALID (more than the hold; hold still active)
4,CAPTURE,a1,50 → OK (balance 50, held 0)
5,AUTH,a2,acc1,50 → OK (available was 50)
6,CAPTURE,a1 → INVALID (already captured)

Part 4: Hold expiry (test cases 12-14)
Holds don’t last forever. A hold created at time t expires at time t + holdSeconds. Any request with timestamp >= the expiry sees that hold as expired, with its funds free again, so apply expiry before processing each request. CAPTURE or VOID on an expired auth returns EXPIRED (every time it is attempted), and the balance is unchanged.

Example: holdSeconds = 10

1,DEPOSIT,acc1,100 → OK
2,AUTH,a1,acc1,80 → OK (expires at 12)
5,AUTH,a2,acc1,30 → DECLINED (available is 20)
12,AUTH,a2,acc1,30 → OK (a1 has expired, so available was 100)
13,CAPTURE,a1 → EXPIRED
14,CAPTURE,a2 → OK (balance 70)

Part 5: Refunds and balance queries (test cases 15-17)
REFUND,authId,amount returns amount to the account that the auth belongs to, 
increasing its balance. The total refunded for an auth must not exceed the amount captured on it.
 If it would, or if the auth was never captured or is unknown, 
 return INVALID. Otherwise return OK. Refunds are allowed regardless of expiry. BALANCE,accountId
  returns the string balance,available. An unknown account returns 0,0.

Example: holdSeconds = 100

1,DEPOSIT,acc1,100 → OK
2,AUTH,a1,acc1,60 → OK
3,CAPTURE,a1,40 → OK (balance 60)
4,REFUND,a1,30 → OK (balance 90)
5,REFUND,a1,20 → INVALID (30 + 20 > 40 captured)
6,REFUND,a1,10 → OK (balance 100)
7,AUTH,a2,acc1,50 → OK
8,BALANCE,acc1 → 100,50

Input Format for Custom Testing
Input from stdin is processed as follows and passed to the function.

The first line contains the integer holdSeconds.
The second line contains the integer n, the size of the array requests.
Each of the next n lines contains a string requests[i], in one of the schemas above.

*/
#include<bits/stdc++.h>
using namespace std ; 

vector<string> process(int holdSeconds, vector<string> requests){
    map<string,int>balanceMap ; 
    map<pair<string,string>,int>holdAmountMap ; 
    map<string,int>holdTotalAmount ; 
    map<string,string>authAccountMap ; 
    map<pair<string,string>,int>holdExpiryMap ;
    set<string>expiredAuths ;
    map<string,string>authToAccount ;                    
    map<string,int>capturedMap ;                                
    map<string,int>refundedMap ;                                
    vector<string>logger ; 
    for(int i = 0 ; i < requests.size() ; i++){
        string req = requests[i] ; 
        stringstream ss(req) ; 
        string type,timestamp ; 
        getline(ss,timestamp,',') ;
        getline(ss,type,',') ;

        for(auto it = holdExpiryMap.begin() ; it != holdExpiryMap.end() ; ){
            string eAuth = it->first.first ; 
            string eAcc = it->first.second ; 
            if(!authAccountMap.count(eAuth)){
                it = holdExpiryMap.erase(it) ; 
            }
            else if(it->second <= stoi(timestamp)){
                holdTotalAmount[eAcc] -= holdAmountMap[{eAuth,eAcc}] ;
                holdAmountMap.erase({eAuth,eAcc}) ;
                authAccountMap.erase(eAuth) ;
                if(holdTotalAmount[eAcc]==0){
                    holdTotalAmount.erase(eAcc) ;
                }
                expiredAuths.insert(eAuth) ; 
                it = holdExpiryMap.erase(it) ; 
            }
            else it++ ; 
        }

        int commaCount  = 0 ; 
        if(type=="CAPTURE"){
            for(int j = 0 ; j< req.size() ; j++){
                if(req[j]==','){
                    commaCount++ ; 
                }
            }
        }
        if(type=="DEPOSIT"){
            string accountId, amount ; 
            getline(ss,accountId,',') ;
            getline(ss,amount,',') ;
            balanceMap[accountId] += stoi(amount) ;
            logger.push_back("OK") ; 
        }
        if(type=="AUTH"){
            string authId,accountId,amount ; 
            getline(ss,authId,',') ;
            getline(ss,accountId,',') ; 
            getline(ss,amount,',') ; 
            int holdedAmount = 0 ; 
            if(holdTotalAmount.count(accountId)){
                 holdedAmount = holdTotalAmount[accountId] ;
            }
            if(balanceMap[accountId] -holdedAmount < stoi(amount)){
                logger.push_back("DECLINED") ;
                continue ; 
            }
            holdTotalAmount[accountId] += stoi(amount) ;
            holdAmountMap[{authId,accountId}] = stoi(amount) ; 
            holdExpiryMap[{authId,accountId}] = stoi(timestamp) + holdSeconds ;
            logger.push_back("OK") ; 
            authAccountMap[authId] = accountId ;
            authToAccount[authId] = accountId ;                 
        }
        if(type=="CAPTURE" && commaCount==2){
            string authId  ; 
            getline(ss,authId,',') ;
            if(expiredAuths.count(authId)){
                logger.push_back("EXPIRED") ; 
                continue ;
            }
            if(!authAccountMap.count(authId)){
                logger.push_back("INVALID") ;
                continue ; 
            }
            string accountId = authAccountMap[authId] ; 
            int capturedAmount = holdAmountMap[{authId,accountId}] ;
            holdTotalAmount[accountId]-=capturedAmount ;
            holdAmountMap.erase({authId,accountId}) ;
            authAccountMap.erase(authId);
            balanceMap[accountId]-=capturedAmount ;
            capturedMap[authId] = capturedAmount ;              
            if(holdTotalAmount[accountId]==0){
                holdTotalAmount.erase(accountId) ;
            }
            logger.push_back("OK") ; 
        }
        if(type=="CAPTURE" && commaCount==3){
            string authId,amount ; 
            getline(ss,authId,',') ;
            getline(ss,amount,',') ;
            if(expiredAuths.count(authId)){
                logger.push_back("EXPIRED") ; 
                continue ;
            }
            if(!authAccountMap.count(authId)){
                logger.push_back("INVALID");
                continue;
            }
            string accountId = authAccountMap[authId] ; 
            if(holdAmountMap[{authId,accountId}]<stoi(amount)){
                logger.push_back("INVALID") ; 
                continue ;
            }
            balanceMap[accountId] -= stoi(amount) ;       
            capturedMap[authId] = stoi(amount) ;                
            holdTotalAmount[accountId] -= holdAmountMap[{authId,accountId}] ;
            holdAmountMap.erase({authId,accountId}) ;
            authAccountMap.erase(authId) ;                 
            if(holdTotalAmount[accountId]==0){
                holdTotalAmount.erase(accountId) ;
            }
            logger.push_back("OK") ; 
        }
        if(type=="VOID"){
            string authId ; 
            getline(ss,authId,',') ;
            if(expiredAuths.count(authId)){
                logger.push_back("EXPIRED") ; 
                continue ;
            }
            if(!authAccountMap.count(authId)){
                logger.push_back("INVALID") ;
                continue ; 
            }
            string accountId = authAccountMap[authId] ; 
            holdTotalAmount[accountId] -= holdAmountMap[{authId,accountId}] ;
            holdAmountMap.erase({authId,accountId}) ;
            authAccountMap.erase(authId) ; 
            if(holdTotalAmount[accountId]==0){
                holdTotalAmount.erase(accountId) ;
            }
            logger.push_back("OK") ;
        }

        if(type=="REFUND"){
            string authId,amount ;
            getline(ss,authId,',') ;
            getline(ss,amount,',') ;
            if(!capturedMap.count(authId)){                     
                logger.push_back("INVALID") ;
                continue ;
            }
            if(refundedMap[authId] + stoi(amount) > capturedMap[authId]){   
                logger.push_back("INVALID") ;
                continue ;
            }
            string accountId = authToAccount[authId] ;
            balanceMap[accountId] += stoi(amount) ;             
            refundedMap[authId] += stoi(amount) ;
            logger.push_back("OK") ;
        }

        if(type=="BALANCE"){
            string accountId ;
            getline(ss,accountId,',') ;
            int bal = 0 , held = 0 ;
            if(balanceMap.count(accountId)) bal = balanceMap[accountId] ;
            if(holdTotalAmount.count(accountId)) held = holdTotalAmount[accountId] ;
            logger.push_back(to_string(bal) + "," + to_string(bal - held)) ;
        }

    }
    return logger ;
}
/*
Question 2: API Rate Limiter

Stripe’s public API serves millions of merchants. Each merchant’s server calls Stripe’s API routes, such as /v1/charges (create a payment) or /v1/refunds (refund a payment). To keep one merchant from overloading the system, each merchant is throttled.

For this multi-part question, we will write a “rate limiter”. We won’t serve any real API calls. Instead, you will implement a function rate_limit that decides, for each incoming API call, whether it is ALLOWED or REJECTED. The implementation is broken down into five parts.

Terms used throughout

merchant: a Stripe customer, identified by merchantId (e.g. m1). Limits are tracked per merchant. One merchant’s traffic never affects another’s.
endpoint: the API route being called (e.g. /charges). It is not a server. Endpoint limits are tracked per (merchant, endpoint) pair. If the limit for /refunds is 1, then merchant m1 may call /refunds once per window and merchant m2 may also call /refunds once per window, independently.
window: the last windowSeconds seconds. At time t, the window is (t - windowSeconds, t], so t - windowSeconds is excluded and t is included.
Only allowed requests count toward any limit. A rejected request is not counted.

Function Description
Complete the function rate_limit in the editor below.

Function Parameters

int maxRequests: the base number of requests a merchant may make in any window.
int windowSeconds: the length of the window.
int blockSeconds: how long a merchant stays blocked after repeated rejections (used in Part 4).
string endpointLimits[k]: configuration lines with the schema endpoint,limit, meaning “each merchant may call endpoint at most limit times per window” (used in Part 2).
string tiers[m]: configuration lines with the schema merchantId,multiplier, meaning “this merchant’s limit is maxRequests * multiplier” (used in Part 3).
string endpointCosts[c]: configuration lines with the schema endpoint,cost, meaning “one call to endpoint counts as cost units instead of 1” (used in Part 5).
string requests[n]: the incoming API calls, each with the schema timestamp,merchantId,endpoint. Example: 7,m1,/charges means merchant m1 called /charges at second 7. Timestamps are non-decreasing integers.

Returns
string[n]: one entry per request, in the same order, either ALLOWED or REJECTED.

Part 1: Basic sliding window (test cases 1-4)
A request is ALLOWED if its merchant has fewer than maxRequests allowed requests (to any endpoint) in the window. Otherwise it is REJECTED.

Example: maxRequests = 2, windowSeconds = 10

1,m1,/charges → ALLOWED
2,m1,/charges → ALLOWED
3,m1,/charges → REJECTED (m1 already has 2 in window)
11,m1,/charges → ALLOWED (window (1,11] contains only t=2)
12,m1,/charges → ALLOWED (window (2,12] contains only t=11)

Part 2: Per-endpoint limits (test cases 5-8)
Some endpoints are more expensive for Stripe than others, so they get a stricter limit. A request must now pass both checks:

the merchant-wide limit from Part 1 (all of this merchant’s endpoints combined), and
the endpoint limit for this merchant on this endpoint, if endpoint appears in endpointLimits. It counts only this merchant’s allowed calls to this same endpoint in the window.

Endpoints not listed in endpointLimits have only the merchant-wide limit. A request that fails either check is REJECTED.

Example: maxRequests = 3, windowSeconds = 10, endpointLimits = ["/refunds,1"]

1,m1,/refunds → ALLOWED
2,m1,/refunds → REJECTED (m1’s /refunds limit is 1)
2,m2,/refunds → ALLOWED (m2 has its own count for /refunds)
3,m1,/charges → ALLOWED
4,m1,/charges → ALLOWED
5,m1,/charges → REJECTED (merchant-wide: m1 has t=1, 3, 4 in the window)

Part 3: Merchant tiers (test cases 9-11)
Some merchants pay for a higher tier. The merchant-wide limit for merchant m becomes maxRequests * multiplier. Merchants not listed in tiers use a multiplier of 1. Endpoint limits from Part 2 are not multiplied.

Example: maxRequests = 1, windowSeconds = 10, tiers = ["m2,3"]

1,m1,/a → ALLOWED
2,m2,/a → ALLOWED
3,m2,/a → ALLOWED
4,m2,/a → ALLOWED
5,m2,/a → REJECTED (m2’s limit is 3 and it has 3 in the window)
6,m1,/a → REJECTED (m1’s limit is 1 and t=1 is in the window)



Part 4: Penalty box (test cases 12-14)
A merchant that keeps hammering the API after being rejected gets blocked.
Track each merchant’s current streak of consecutive rejections caused by the limits. An ALLOWED request resets the streak to 0.
When the streak reaches 3 at time t, the merchant is blocked. Every request from that merchant with timestamp < t + blockSeconds is REJECTED.
Blocked requests do not count toward any limit and do not extend the streak.
When the block ends, the streak resets to 0.
Example: maxRequests = 1, windowSeconds = 10, blockSeconds = 10
1,m1,/a → ALLOWED
2,m1,/a → REJECTED (streak 1)
3,m1,/a → REJECTED (streak 2)
4,m1,/a → REJECTED (streak 3, blocked until time 14)
5,m1,/a → REJECTED (blocked)
13,m1,/a → REJECTED (blocked)
14,m1,/a → ALLOWED (block over; window (4,14] has no allowed requests


Part 5: Weighted cost (test cases 15-17)
Not every call is equal. Each endpoint has a cost, 1 by default, or the value in endpointCosts. The merchant-wide limit and the endpoint limits now apply to the sum of costs of allowed requests in the window, not to their count. A request with cost c is allowed only if currentSum + c <= limit for the merchant-wide limit and for its endpoint limit (if any). A request whose cost alone exceeds a limit is always rejected.
Example: maxRequests = 5, windowSeconds = 10, endpointCosts = ["/export,4"]
1,m1,/export → ALLOWED (sum 4)
2,m1,/search → ALLOWED (sum 5)
3,m1,/search → REJECTED (5 + 1 > 5)
4,m1,/export → REJECTED (5 + 4 > 5)
11,m1,/export → ALLOWED (window (1,11] holds only t=2 with cost 1; 1 + 4 = 5)

Input Format for Custom Testing
Input from stdin is processed as follows and passed to the function.

The first line contains the integer maxRequests.
The second line contains the integer windowSeconds.
The third line contains the integer blockSeconds.
The next line contains k, the size of endpointLimits, followed by k lines.
The next line contains m, the size of tiers, followed by m lines.
The next line contains c, the size of endpointCosts, followed by c lines.
The next line contains n, the size of requests, followed by n lines.
 */
#include<bits/stdc++.h>
using namespace std ; 
void onReject(const string &m, int ts, int blockSeconds,
              map<string,int> &blockedMerchants,
              map<string,int> &blockMerchentEndPoint){
    blockedMerchants[m] += 1 ;
    if(blockedMerchants[m] == 3){
        blockMerchentEndPoint[m] = ts + blockSeconds ;
    }
}

vector<string> rate_limit(int maxRequests, int windowSeconds, int blockSeconds, vector<string> endpointLimits, vector<string> tiers,
vector<string> endpointCosts, vector<string> requests){
    int n = requests.size() ;
    map<string,int>endpointLimitsMap ;
    map<string,deque<pair<int,int>>>slidingWindowPerMerchent ; 
    map<pair<string,string>,deque<pair<int,int>>>endpointCostMap ;
    map<string,int>blockedMerchants ; 
    map<string,int>blockMerchentEndPoint ;   
    map<string,int>tiersMap ; 
    map<string,int>endpointCostsMap ;                                          
    map<string,int>merchantSum ;                                               
    map<pair<string,string>,int>endpointSum ; 
    for(auto s : endpointCosts){                       // CHANGED: new loop, fills endpointCostsMap
        stringstream ss(s) ;
        string endpoint,cost ;
        getline(ss,endpoint,',') ;
        getline(ss,cost,',') ;
        endpointCostsMap[endpoint] = stoi(cost) ;
    }
    for(auto s : tiers){
        stringstream ss(s) ; 
        string merchent,tier ;
        getline(ss,merchent,',') ;
        getline(ss,tier,',') ;
        tiersMap[merchent] = stoi(tier) ;
    }
    vector<string>ans ; 
    for(auto s : endpointLimits){
        stringstream ss(s) ; 
        string endpoint,limit ;
        getline(ss,endpoint,',') ;
        getline(ss,limit,',') ;
        endpointLimitsMap[endpoint] = stoi(limit) ;
    }
    for(int i = 0  ; i< n ; i++){
        string req = requests[i] ; 
        stringstream ss(req) ;
        string time,merchent,type ; 
        getline(ss,time,',') ;
        getline(ss,merchent,',') ;
        getline(ss,type,',') ;
            int timestamp = stoi(time) ;
            int startTime = max(timestamp-windowSeconds+1,0) ; 
            int cost = endpointCostsMap.count(type) ? endpointCostsMap[type] : 1 ;
            if(blockMerchentEndPoint.count(merchent) && blockMerchentEndPoint[merchent]  > timestamp){
                    ans.push_back("REJECTED") ;
                    continue ;
            }   
            if(blockMerchentEndPoint.count(merchent) && blockMerchentEndPoint[merchent] <= timestamp){
                blockedMerchants[merchent] = 0 ;
                blockMerchentEndPoint.erase(merchent) ;
            }
            while(!slidingWindowPerMerchent[merchent].empty() && slidingWindowPerMerchent[merchent].front().first < startTime){
                merchantSum[merchent] -= slidingWindowPerMerchent[merchent].front().second ;
                slidingWindowPerMerchent[merchent].pop_front() ;
            }
            while(!endpointCostMap[{merchent,type}].empty() && endpointCostMap[{merchent,type}].front().first<startTime){
                endpointSum[{merchent,type}] -= endpointCostMap[{merchent,type}].front().second ;
                endpointCostMap[{merchent,type}].pop_front() ;
            }
                 if(tiersMap.count(merchent)){
                    if(merchantSum[merchent] + cost > tiersMap[merchent]*maxRequests){                                          
                        ans.push_back("REJECTED") ;
                        onReject(merchent, timestamp, blockSeconds, blockedMerchants, blockMerchentEndPoint) ;
                        continue ; 
                    }
                }
                else if(merchantSum[merchent] + cost > maxRequests){                                                             
                    ans.push_back("REJECTED") ;
                    onReject(merchent, timestamp, blockSeconds, blockedMerchants, blockMerchentEndPoint) ;
                    continue ; 
                }
            if(endpointLimitsMap.count(type) && endpointSum[{merchent,type}] + cost  > endpointLimitsMap[type]){
                ans.push_back("REJECTED") ;
                onReject(merchent, timestamp, blockSeconds, blockedMerchants, blockMerchentEndPoint) ;  
                continue ;
            }
            slidingWindowPerMerchent[merchent].push_back({timestamp,cost}) ;
            endpointCostMap[{merchent,type}].push_back({timestamp,cost}) ;
            blockedMerchants[merchent] = 0 ;
            merchantSum[merchent] += cost ;
            endpointSum[{merchent,type}] += cost ; 
            ans.push_back("ALLOWED") ;
    }
    return ans ; 
}
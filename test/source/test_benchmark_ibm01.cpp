#include <doctest/doctest.h>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <cstdint>
#include <iostream>
#include <netlistx/netlist.hpp>
#include <random>
#include <string_view>
#include <vector>
#include <ckpttn/FMBiConstrMgr.hpp>
#include <ckpttn/FMBiGainMgr.hpp>
#include <ckpttn/FMConstrMgr.hpp>
#include <ckpttn/FMKWayConstrMgr.hpp>
#include <ckpttn/FMKWayGainMgr.hpp>
#include <ckpttn/FMPartMgr.hpp>
#include <ckpttn/MLPartMgr.hpp>
using namespace std;
#ifdef _MSC_VER
#pragma warning(disable: 4244)
#endif
extern auto readNetD(string_view) -> SimpleNetlist;
extern void readAre(SimpleNetlist&, string_view);
static constexpr int SEED = 47, RUNS = 5;
static constexpr double BAL_TOL = 0.45;
static constexpr size_t LIMIT = 10;
using BiPM = FMPartMgr<SimpleNetlist,FMBiGainMgr<SimpleNetlist>,FMBiConstrMgr<SimpleNetlist>>;
using KwPM = FMPartMgr<SimpleNetlist,FMKWayGainMgr<SimpleNetlist>,FMKWayConstrMgr<SimpleNetlist>>;

static auto run_fm(const SimpleNetlist& h, span<uint8_t> p, uint8_t k) -> pair<int,double>{
    auto t0=chrono::steady_clock::now();
    if(k==2){
        FMBiGainMgr<SimpleNetlist> gm(h); FMBiConstrMgr<SimpleNetlist> cm(h,BAL_TOL);
        BiPM pm(h,gm,cm); if(pm.legalize(p)!=LegalCheck::AllSatisfied) return{-1,0};
        pm.optimize(p); auto t1=chrono::steady_clock::now(); return{pm.total_cost,chrono::duration<double>(t1-t0).count()};}
    FMKWayGainMgr<SimpleNetlist> gm(h,k); FMKWayConstrMgr<SimpleNetlist> cm(h,BAL_TOL,k);
    KwPM pm(h,gm,cm,k); if(pm.legalize(p)!=LegalCheck::AllSatisfied) return{-1,0};
    pm.optimize(p); auto t1=chrono::steady_clock::now(); return{pm.total_cost,chrono::duration<double>(t1-t0).count()};}

static auto run_ml(const SimpleNetlist& h, span<uint8_t> p, uint8_t k) -> pair<int,double>{
    auto t0=chrono::steady_clock::now();
    if(k==2){MLPartMgr pm{BAL_TOL};pm.set_limitsize(LIMIT);
        if(pm.run_Partition<SimpleNetlist,BiPM>(h,p)!=LegalCheck::AllSatisfied)return{-1,0};
        auto t1=chrono::steady_clock::now();return{pm.total_cost,chrono::duration<double>(t1-t0).count()};}
    MLPartMgr pm{BAL_TOL,k};pm.set_limitsize(LIMIT);
    if(pm.run_Partition<SimpleNetlist,KwPM>(h,p)!=LegalCheck::AllSatisfied)return{-1,0};
    auto t1=chrono::steady_clock::now();return{pm.total_cost,chrono::duration<double>(t1-t0).count()};}

TEST_CASE("Benchmark all"){
    struct TC{const char*net;const char*are;};
    TC cs[]={{"../../testcases/ibm01.net","../../testcases/ibm01.are"},{"../../testcases/p1.net",nullptr}};
    cout<<"\n=== ML vs FM (bal_tol="<<BAL_TOL<<" limit="<<LIMIT<<" runs="<<RUNS<<") ===\n";
    for(auto&c:cs){
        auto h=readNetD(c.net);if(c.are)readAre(h,c.are);
        cout<<"\n--- "<<c.net<<" ---\n";
        for(auto k:{2,3,5}){
            auto N=h.number_of_modules();
            cout<<"  K="<<(int)k<<" ("<<N<<" mods)\n";
            vector<int>fc,mc;vector<double>ft,mt;
            for(int r=0;r<RUNS;++r){
                mt19937 rg(SEED+r);
                auto pt=vector<uint8_t>(N,0);
                if(k==2){bernoulli_distribution d(0.5);for(size_t i=0;i<N;++i)pt[i]=(uint8_t)d(rg);}
                else{uniform_int_distribution<int> d(0,k-1);for(size_t i=0;i<N;++i)pt[i]=(uint8_t)d(rg);}
                auto p=pt;auto cr=run_fm(h,p,k);fc.push_back(cr.first);ft.push_back(cr.second);
                p=pt;cr=run_ml(h,p,k);mc.push_back(cr.first);mt.push_back(cr.second);}
            auto a=[](auto&v){return accumulate(v.begin(),v.end(),0.0)/v.size();};
            double af=a(fc),am=a(mc);
            cout<<"    FM="<<af<<"("<<a(ft)<<"s) ML="<<am<<"("<<a(mt)<<"s) impr="<<(af-am)/af*100<<"%\n";}}
    CHECK(true);}

set I;
set PEV;
set C;


param demand{I};
param renewableEnergyProduction{I};
param priceBuying{I};
param priceSelling{I};

param pevDisp{PEV, I};
param pevArrival{PEV, I};
param pevEnergyOnArrival{PEV, I};
param pevDep{PEV, I};
param pevEnergyWishDep{PEV, I};
param pevMinDoD{PEV};
param pevPower{PEV};



param pevDischargePrice{PEV,C};
param pevDischargeRate{PEV,C};
param pevChargeRate{PEV,C};
param pevChargePrice{PEV,C};



var energySelling{i in I}, >= 0 ;
var energyBuying{i in I}, >= 0;
var energySellingActive{i in I}, binary ;
var energyBuyingActive{i in I}, binary ;
var yCharge{v in PEV, c in C, i in I},binary;
var yDischarge{v in PEV, c in C, i in I},binary;
var yBaterryRate{v in PEV, i in I}, >=0, <=100;
var totalChargingDischargingPayed, >=0;
var objCost;
var objWearTear;
var objMaxLoad, >=0;

param gridRateBuying{i in I}  default if (demand[i] - renewableEnergyProduction[i]) > 0 then demand[i] - renewableEnergyProduction[i] else 0;
param gridRateSelling{i in I} default if (renewableEnergyProduction[i] - demand[i]) > 0 then renewableEnergyProduction[i] - demand[i] else 0;
param BIGM{i in I} default if (gridRateBuying[i]>0) then (10*(sum{v in PEV} pevPower[v]) + gridRateBuying[i]) else 10*(sum{v in PEV} pevPower[v]) + gridRateSelling[i];

s.t. 

calcEnergyEachInterval{i in I}:  sum{v in PEV} sum{c in C} (yDischarge[v,c,i]*pevDischargeRate[v,c]*pevPower[v]/100 - yCharge[v,c,i]*pevChargeRate[v,c]*pevPower[v]/100 )  + renewableEnergyProduction[i] - demand[i] = energySelling[i] - energyBuying[i];


onlyOneAction{v in PEV, i in I}:  sum{c in C} (yDischarge[v,c,i] +  yCharge[v,c,i]) <= 1;
onlyDischargeIfAvailable{v in PEV, i in I}:  sum{c in C} yDischarge[v,c,i] <= pevDisp[v,i];
onlyChargeIfAvailable{v in PEV, i in I}:  sum{c in C} yCharge[v,c,i] <= pevDisp[v,i];

calcBaterryRateFirstInterval{v in PEV}: yBaterryRate[v,1] = pevEnergyOnArrival[v,1]*pevArrival[v,1] + sum{c in C} (yCharge[v,c,1]*pevChargeRate[v,c] - yDischarge[v,c,1]*pevDischargeRate[v,c]);
calcBaterryRate{v in PEV, i in I:i>=2}: yBaterryRate[v,i] = ( (1-pevArrival[v,i])*yBaterryRate[v,i-1] + pevArrival[v,i]*pevEnergyOnArrival[v,i] + 
sum{c in C} (yCharge[v,c,i]*pevChargeRate[v,c] - yDischarge[v,c,i]*pevDischargeRate[v,c]) )*( pevDisp[v,i] + pevDep[v,i]);

attendVehicleEnergyWish{v in PEV, i in I}: yBaterryRate[v,i] >= pevEnergyWishDep[v,i]*pevDep[v,i];

attendMinDoD{v in PEV, i in I}: yBaterryRate[v,i] >=  pevMinDoD[v]*pevDisp[v,i];

sellingActive{i in I}: energySellingActive[i]*BIGM[i] >= energySelling[i];
buyingActive{i in I}: energyBuyingActive[i]*BIGM[i] >= energyBuying[i];
sellingOrBuying{i in I}: (energySellingActive[i]+energyBuyingActive[i]) <= 1;

calcTotalDischargePrice: totalChargingDischargingPayed = sum{i in I} sum{v in PEV} sum{c in C} (yDischarge[v,c,i]*pevDischargePrice[v,c]*pevPower[v]/100 + yCharge[v,c,i]*pevChargePrice[v,c]*pevPower[v]/100 );


obj1TotalCost: 
	objCost = sum{i in I} (energyBuying[i]*priceBuying[i] - energySelling[i]*priceSelling[i]) + totalChargingDischargingPayed;

obj2WearAndTear: 
        objWearTear = sum{i in I} sum{v in PEV} sum{c in C} (yDischarge[v,c,i]*pevDischargeRate[v,c]*pevPower[v]/100 + 
	yCharge[v,c,i]*pevChargeRate[v,c]*pevPower[v]/100);

obj3CalcMaxLoad1{i in I}: 
	objMaxLoad >= energySelling[i] + energyBuying[i]; 

minimize totalPrice:  objCost + objWearTear + objMaxLoad ;



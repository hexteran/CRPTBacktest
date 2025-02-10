# -*- coding: utf-8 -*-
"""
Spyder Editor

This is a temporary script file.
"""

from my_module import *
        # Define the callback functions that the C++ simulation will call.

class Strategy:
    def __init__(self, data: list, execution_latency, market_data_latency):
        # Create an instance of PyStrategy with the provided market data, latency values, and callbacks.
        self.py_strategy = PyStrategy(
            data,
            execution_latency,
            market_data_latency,
            self.OnOrderFilled,
            self.OnOrderCanceled,
            self.OnOrderModified,
            self.OnNewOrder,
            self.OnTrade
        )
        self.sent = False
        
        # A list to store orders created by the strategy.
        self.orders = []
    
    def OnOrderFilled(self, order):
        print('filled')
    
    def OnOrderCanceled(self, order):
        pass
    
    def OnOrderModified(self, order):
        pass
    
    def OnNewOrder(self, order):
        print('new')
    
    def OnTrade(self, trade):
        print(trade.Instrument)
        if (self.sent == False):
            self.sent = True
            self.SendOrder('TestInstrument', 0, 1, Side.Buy, OrderType.Market)

    def SendOrder(self, instrument: str, price: float, qty: float, order_side: Side, order_type: OrderType):
        # Create a new Order instance.
        new_order = Order()
        new_order.Instrument = instrument
        new_order.Price = price
        new_order.Qty = qty
        new_order.OrderSide = order_side
        new_order.Type = order_type

        # Save the order in our list.
        self.orders.append(new_order)
        self.py_strategy.send_order(new_order)

    def CancelOrder(self, order: Order):
        self.py_strategy.cancel_order(order)
    
    def GetFilledOrders(self):
        result = []
        for order in self.orders:
            if order.State == OrderState.Filled:
                result.append({'instrument': order.Instrument, 
                               'nominal_price': order.Price, 
                               'exec_price': order.LastExecPrice,
                               'qty': order.Qty,
                               'filled_qty': order.FilledQty})
        return result
    
    def Run(self):
        self.py_strategy.run()

import pandas as pd

test = pd.read_csv("//home//hexteran//git//CRPTBacktest//data//simulation_test_trades_5.csv", header = None)
data = []

test = test.to_dict(orient = 'list')
#print(test)
for i in range(len(test[0])):
    trade = MDTrade()
    trade.EventTimestamp = test[0][i]
    trade.Price = test[1][i]
    trade.Qty = test[2][i]
    trade.AggressorSide = Side.Buy if test[3][i].lower() == 'buy' else Side.Sell
    trade.Instrument = test[4][i]
    data.append(trade)

strat = Strategy(data, 0, 0)
strat.Run()

print(strat.GetFilledOrders())
    

        

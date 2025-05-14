# -*- coding: utf-8 -*-

import os
import sys

# Determine the absolute path to the shared library directory.
lib_dir = os.path.join(os.path.dirname(__file__), 'libs')
if lib_dir not in os.environ.get('LD_LIBRARY_PATH', ''):
    os.environ['LD_LIBRARY_PATH'] = lib_dir + os.pathsep + os.environ.get('LD_LIBRARY_PATH', '')
    os.environ['PYTHONPATH'] = lib_dir + os.pathsep + os.environ.get('PYTHONPATH', '')

import datetime
from python_simulator import *

class DataStorage:
    def __init__(self):
        self.storage = PyDataStorage()

    def AddVMDTrades(self, rowName: str,
        timestamps: list[int],
        prices:list[float],
        qtys: list[float],
        sides: list[Side],
        instruments: list[str]):
        assert(len(timestamps) == len(prices) == len(qtys) == len(sides) == len(instruments))
        self.storage.add_v_md_trades(rowName, timestamps, prices, qtys, sides, instruments)

    def AddVMDL1Updates(self, rowName: str,
        timestamps: list[int],
        askPrices:list[float],
        askQtys: list[float],
        bidPrices:list[float],
        bidQtys: list[float],
        instruments: list[str]):
        assert(len(timestamps) == len(askPrices) == len(askQtys) == len(bidPrices) == len(bidQtys))
        self.storage.add_v_md_l1_updates(rowName, timestamps, askPrices, askQtys, bidPrices, bidQtys, instruments)

    def AddVMDCustomUpdates(self, rowName: str,
        timestamps: list[int],
        texts:list[str],
        payloads: list[float]):
        assert(len(timestamps) == len(texts) == len(payloads))
        self.storage.add_v_md_custom_updates(rowName, timestamps, texts, payloads)

    def AddVMDCustomMultipleUpdates(self, rowName: str,
        timestamps: list[int],
        texts:list[str],
        payloads: list[dict]):
        assert(len(timestamps) == len(texts) == len(payloads))
        self.storage.add_v_md_custom_multiple_updates(rowName, timestamps, texts, payloads)

    def AddMDTrades(self, md_trades: dict):
        self.storage.add_md_trades(md_trades)

    def AddL1Updates(self, md_updates: dict):
        self.storage.add_md_l1_updates(md_updates)

    def AddMDCustomUpdates(self, custom_updates: dict):
        self.storage.add_md_custom_updates(custom_updates)

    def AddMDCustomMultipleUpdates(self, custom_multiple_updates: dict):
        self.storage.add_md_custom_multiple_updates(custom_multiple_updates)

class Strategy:
    def __init__(self, execution_latency: int, market_data_latency: int, dataStorage = None):
        if dataStorage is None:
            # Create an instance of PyStrategy with the provided market data, latency values, and callbacks.
            self.py_strategy = PyStrategy(
                execution_latency,
                market_data_latency,
                self.OnOrderFilled,
                self.OnOrderCanceled,
                self.OnOrderModified,
                self.OnNewOrder,
                self.OnTrade,
                self.OnL1Update,
                self.OnCustomUpdate,
                self.OnCustomMultipleUpdate
            )  

        else:
            self.py_strategy = PyStrategy(
                dataStorage.storage,
                execution_latency,
                market_data_latency,
                self.OnOrderFilled,
                self.OnOrderCanceled,
                self.OnOrderModified,
                self.OnNewOrder,
                self.OnTrade,
                self.OnL1Update,
                self.OnCustomUpdate,
                self.OnCustomMultipleUpdate
            )  
              
        # A list to store orders created by the strategy.
        self.orders = []
    
    def AddMDTrades(self, md_trades: dict):
        self.py_strategy.add_md_trades(md_trades)
        
    def AddMDCustomUpdates (self, md_custom_updates: dict):
        self.py_strategy.add_md_custom_updates(md_custom_updates)
    
    def AddMDCustomMultipleUpdates (self, md_custom_multiple_updates: dict):
        self.py_strategy.add_md_custom_multiple_updates(md_custom_multiple_updates)
        
    def OnOrderFilled(self, order):
        pass
    
    def OnOrderCanceled(self, order):
        pass
    
    def OnOrderModified(self, order):
        pass
    
    def OnNewOrder(self, order):
        pass
    
    def OnTrade(self, trade):
        pass
    
    def OnL1Update(self, update):
        pass

    def OnCustomUpdate(self, custom_update):
        pass
    
    def OnCustomMultipleUpdate(self, custom_multiple_update):
        pass
        

    def SendOrder(self, instrument: str, price: float, qty: float, order_side: Side, order_type: OrderType, text = ""):
        # Create a new Order instance.
        new_order = Order()
        new_order.Instrument = instrument
        new_order.Price = price
        new_order.Qty = qty
        new_order.OrderSide = order_side
        new_order.Type = order_type
        new_order.Text = text

        # Save the order in our list.
        self.orders.append(new_order)
        self.py_strategy.send_order(new_order)
        
        return new_order

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
                               'state': order.State,
                               'filled_qty': order.FilledQty,
                               'create_timestamp': order.CreateTimestamp,
                               'last_report_timestamp': order.LastReportTimestamp,
                               'side': 'BUY' if order.OrderSide == Side.Buy else 'SELL',
                               'type': 'Limit' if order.Type == OrderType.Limit else 'Market',
                               'text': order.Text})
        return result

    def GetOrders(self):
        result = []
        for order in self.orders:
            result.append({'instrument': order.Instrument,
                            'nominal_price': order.Price, 
                            'exec_price': order.LastExecPrice,
                            'state': order.State,
                            'qty': order.Qty,
                            'filled_qty': order.FilledQty,
                            'create_timestamp': order.CreateTimestamp,
                            'last_report_timestamp': order.LastReportTimestamp,
                            'side': 'BUY' if order.OrderSide == Side.Buy else 'SELL',
                            'type': 'Limit' if order.Type == OrderType.Limit else 'Market',
                            'text': order.Text})
        return result
    
    def Run(self):
        self.py_strategy.run()

    def CommitData(self):
        self.py_strategy.commit_data()
    
def ns_to_datetime(ns):
    seconds = ns / 1e9
    return datetime.datetime.fromtimestamp(seconds)

        

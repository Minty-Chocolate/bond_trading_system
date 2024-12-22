import random
import time
from tqdm import tqdm
from itertools import product

# https://www.treasurydirect.gov/auctions/auction-query/
product_ids = [
    "91282CME8",
    "91282CMB4",
    "91282CMD0",
    "91282CMC2",
    "91282CLW9",
    "912810UF3",
    "912810UE6",
]

price_entries = orderbook_entries = int(1e3)

def generate_trades():
    volumes = [1000000, 2000000, 3000000, 4000000, 5000000]
    volume_index = 0

    prices = [99.0, 100.0]
    price_index = 0

    side = 0

    books = ["TRSY1", "TRSY2", "TRSY3"]
    book_index = 0

    with open("input/trades.txt", "w") as input_file:
        for product_id, _ in tqdm(product(product_ids, range(10))):
            trade_id = str(int(time.time() * 1e6))
            input_file.write(
                f"{product_id},{trade_id},{prices[price_index]},{books[book_index]},{volumes[volume_index]},{side}\n"
            )
            volume_index = (volume_index + 1) % len(volumes)
            price_index = (price_index + 1) % len(prices)
            book_index = (book_index + 1) % len(books)
            side = (side + 1) % 2


def generate_inquiries():
    volumes = [1000000, 2000000, 3000000, 4000000, 5000000]
    volume_index = 0

    side = 0

    with open("input/inquiries.txt", "w") as input_file:
        for product_id, _ in tqdm(product(product_ids, range(10))):
            inquiry_id = str(int(time.time() * 1e6))
            input_file.write(
                f"{product_id},{inquiry_id},{side},{volumes[volume_index]}\n"
            )
            volume_index = (volume_index + 1) % len(volumes)
            side = (side + 1) % 2


def generate_prices(num_rows):
    with open("input/prices.txt", "w") as input_file:
        for product_id, _ in tqdm(product(product_ids, range(num_rows))):
            spread = "0-00" + "23+"[random.randint(0, 2)]
            input_file.write(
                f"{product_id},{get_random_fractional_price(99, 101)},{spread}\n"
            )

def generate_marketdata(num_rows):
    volumes = [10000000, 20000000, 30000000, 40000000, 50000000]
    volume_index = 0

    spread = 2
    spread_increment = 2

    input_file = open("input/marketdata.txt", "w")

    with open("input/marketdata.txt", "w") as input_file:
        for product_id, _ in tqdm(product(product_ids, range(num_rows))):
            row = product_id
            mid = random.randint(99 * 256, 101 * 256)
            for k in range(5):
                row += f",{get_fractional_string(mid - spread / 2 - k)},{volumes[volume_index]}"
                volume_index = (volume_index + 1) % len(volumes)
            for k in range(5):
                row += f",{get_fractional_string(mid + spread / 2 + k)},{volumes[volume_index]}"
                volume_index = (volume_index + 1) % len(volumes)

            row += "\n"
            input_file.write(row)

            spread += spread_increment
            if spread == 8 and spread_increment == 2:
                spread_increment = -2
            elif spread == 2 and spread_increment == -2:
                spread_increment = 2


def get_random_fractional_price(low, high):
    first = random.randint(0, 31)
    second = random.randint(0, 7)
    return f"{random.randint(low, high - 1)}-{f'{first}' if first > 9 else f'0{first}'}{'+' if second == 4 else second}"


def get_fractional_string(number):
    quotient, remainder = number / 256, number % 256
    first = remainder / 8
    second = remainder % 8
    return f"{quotient}-{f'{first}' if first > 9 else f'0{first}'}{'+' if second == 4 else second}"




generate_prices(price_entries)
generate_marketdata(orderbook_entries)
generate_trades()
generate_inquiries()
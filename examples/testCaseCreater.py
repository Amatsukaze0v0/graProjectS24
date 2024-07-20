import csv
import random

# 生成测试用例
def generate_test_cases(num_cases):
    test_cases = []
    operations = ['R', 'W']
    for _ in range(num_cases):
        operation = random.choice(operations)
        address = random.randint(0, 1023) * 4  # 确保地址对齐（4字节对齐）
        data = random.randint(0, 2**32 - 1) if operation == 'W' else 0
        test_cases.append([operation, address, data if operation == 'W' else ''])
    return test_cases

# 生成200行测试用例
num_cases = 200
test_cases = generate_test_cases(num_cases)

# 保存为CSV文件
csv_filename = 'input2.csv'
with open(csv_filename, mode='w', newline='') as file:
    writer = csv.writer(file)
    for case in test_cases:
        writer.writerow(case)

print(f"{num_cases} test cases generated and saved to {csv_filename}")

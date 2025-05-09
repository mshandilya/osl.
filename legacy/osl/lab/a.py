def is_palindrome(n):
    rev = 0
    nn = n
    while n > 0:
        rev = rev * 10 + n % 10
        n //= 10
    return rev == nn

def F():
    max_pal = 0
    i = 999
    while i >= 100:
        j = i
        while j >= 100:
            prod = i * j
            if prod > max_pal and is_palindrome(prod):
                max_pal = prod
            j -= 1
        i -= 1
    return max_pal

F()
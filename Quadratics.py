# https://brilliant.org/wiki/lagrange-interpolation/

# create a function which maps a given voltage to a frequency
# frequency = a(voltage^2) + b(voltage) + c

# we determine the three coeffecients by using three known / measurable points of the system (ie. [voltage, frequency])


import math

def calculateA(p1, p2, p3):
  x1, y1 = p1
  x2, y2 = p2
  x3, y3 = p3

  D1 = calculateDenominator(x1, x2, x3)
  D2 = calculateDenominator(x2, x1, x3)
  D3 = calculateDenominator(x3, x1, x2)
  return y1/D1 + y2/D2 + y3/D3

def calculateB(p1, p2, p3):
  x1, y1 = p1
  x2, y2 = p2
  x3, y3 = p3

  D1 = calculateDenominator(x1, x2, x3)
  D2 = calculateDenominator(x2, x1, x3)
  D3 = calculateDenominator(x3, x1, x2)
  return -(y1 * (x2 + x3) / D1) - (y2 * (x1 + x3) / D2) - (y3 * (x1 + x2) / D3)

def calculateC(p1, p2, p3):
  x1, y1 = p1
  x2, y2 = p2
  x3, y3 = p3

  D1 = calculateDenominator(x1, x2, x3)
  D2 = calculateDenominator(x2, x1, x3)
  D3 = calculateDenominator(x3, x1, x2)
  return ((y1 * x2 * x3) / D1) + ((y2 * x1 * x3) / D2) + ((y3 * x1 * x2) / D3)

def calculateDenominator(x1, x2, x3):
  return math.pow(x1, 2) - x1*x3 - x1*x2 + x2*x3

def makeYSolver(p1, p2, p3):
  a = calculateA(p1, p2, p3)
  b = calculateB(p1, p2, p3)
  c = calculateC(p1, p2, p3)
  def ySolver(x):
    return a*math.pow(x, 2) + b*x + c
  return ySolver

def makeXSolver(p1, p2, p3):
  a = calculateA(p1, p2, p3)
  b = calculateB(p1, p2, p3)
  c = calculateC(p1, p2, p3)
  def xSolver(y):
    plus = (-b + math.sqrt(math.pow(b, 2) - 4*a*(c-y))) / 2*a
    minus = (-b - math.sqrt(math.pow(b, 2) - 4*a*(c-y))) / 2*a
    return max(plus, minus)
  return xSolver

p1 = (0.0,0.0)
p2 = (1.0,1.0) 
p3 = (2.0,4.0)

ySolver = makeYSolver(p1, p2, p3)
xSolver = makeXSolver(p1, p2, p3)

print('a: ', calculateA(p1, p2, p3))
print('b: ', calculateB(p1, p2, p3))
print('c: ', calculateC(p1, p2, p3))

print(ySolver(2.0))
print(xSolver(4.0))
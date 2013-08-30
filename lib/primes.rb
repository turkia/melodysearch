# C-Brahms Engine for Musical Information Retrieval
# University of Helsinki, Department of Computer Science
#
# Version 0.2.4, May 15th, 2003
#
# Copyright (C) 2000-01-09 K. Kodama
#
# Contacts: kodama@kobe-kosen.ac.jp
#
# This file contains methods from Number class of
# Poly-Ruby library released under GPL license by K. Kodama.
# They are used to generate primes to be used as hash table sizes for 
# version 1 of SIA(M)E algorithm. 
 
 
module Number

	# Returns prime next to n.
	# This is used to generate primes to be used as hash table sizes for 
	# version 1 of SIA(M)E algorithm. 
	def nextPrime(n)
		# return prime next to n
		if n < 2 then return 2 end
		n = n + 1; 
		if (n % 2 == 0) then n = n + 1 end
		while not prime?(n) do n = n + 2 end
		n
	end

	# Returns true if x is prime, and otherwise false.
	def prime?(x)

		if x & 1 == 0; if x == 2 then return true else return false end
		elsif x % 3 == 0; if x == 3 then return true else return false end
		elsif x % 5 == 0; if x == 5 then return true else return false end
		elsif x % 7 == 0; if x == 7 then return true else return false end
		elsif x % 11 == 0; if x == 11 then return true else return false end
		elsif x % 13 == 0; if x == 13 then return true else return false end
		elsif x < 2; return false
		end

		if (x > 1000000000) && (not fpsp?(x, 13)) then return false end

		d = sqrti(x)
		r1 = 17
		r2 = 19
		while r1 <= d
			if x % r1 == 0 then return false 
			elsif x % r2 == 0 then return false end
			r1 = r1 + 6
			r2 = r2 + 6
		end
		true
	end

	# Integer square root.
	# Returns an integer x s.t. x^2 <= n < (x+1)^2.
	def sqrti(n) 
		n = n.to_i
		if n <= 0 then return 0
		elsif n <= 3 then return 1 end

		x = n >> 1
		y = 2

		while y < x do
			x = (x + y) >> 1
			y = n / x 
		end
		x
	end

	# Tests if x is prime or Fermat pseudo prime of base a.
	def fpsp?(x,a)
		powerI(a, x - 1, x) == 1
	end

	# Returns a^n modulo m, where n >= 0.
	def powerI(a, n, m = 0)
		if m == 0
			s = 1
			while n > 0
				if n[0] == 1 then s = s * a end
				a = a * a
				n >>= 1
			end
			return s
		else
			s = 1
			while n > 0
				if n[0] == 1 then s = (s * a) % m end 
				a = (a * a) % m
				n >>= 1
			end
			return s
		end
	end

	module_function :powerI, :prime?, :nextPrime, :fpsp?, :sqrti
end

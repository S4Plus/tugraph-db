match (n0:Account)-[r1*..2]->(n1:Loan)
match (n1)<-[r2]-(n2)<-[r3]-(n3:Company) return count(n1)
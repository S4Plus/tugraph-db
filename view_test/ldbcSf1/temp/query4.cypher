match (n0:Place)<-[r1*2]-(n1:Comment)
match (n1)-[r2:replyOf*..]->(n2:Post) return count(n2)
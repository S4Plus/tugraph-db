MATCH (acc:Account {id:%d}), (med:Medium {id:%d}) CREATE (acc)<-[:signIn {timestamp: %d}]-(med);
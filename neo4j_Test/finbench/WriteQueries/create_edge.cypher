MATCH (acc:Account {id: $accountId}), (loan: Loan {id: $loanId}) create (acc)<-[:deposit {timestamp: $depositTime, amount: $amt}]-(loan)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DB_FILE "bank_data.txt"
#define TEMP_FILE "bank_temp.txt"

#define MAX_NAME 50
#define MAX_ACCOUNTS 100

typedef struct
{
    int accountNo;
    char name[MAX_NAME];
    double balance;
    int active;
} Account;


/* ---------------------------------------------------------
   UTILITY FUNCTIONS
   --------------------------------------------------------- */

void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


void getDateTime(char *buffer, int size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}


/* ---------------------------------------------------------
   DATABASE - LOAD ACCOUNTS
   --------------------------------------------------------- */

int loadAccounts(Account accounts[])
{
    FILE *file;
    int count = 0;

    file = fopen(DB_FILE, "r");

    if (file == NULL)
    {
        return 0;
    }

    while (fscanf(file, "%d|%49[^|]|%lf|%d\n",
                  &accounts[count].accountNo,
                  accounts[count].name,
                  &accounts[count].balance,
                  &accounts[count].active) == 4)
    {
        count++;

        if (count >= MAX_ACCOUNTS)
            break;
    }

    fclose(file);

    return count;
}


/* ---------------------------------------------------------
   DATABASE - SAVE ACCOUNTS
   --------------------------------------------------------- */

int saveAccounts(Account accounts[], int count)
{
    FILE *file;

    file = fopen(DB_FILE, "w");

    if (file == NULL)
    {
        printf("\nERROR: Cannot open database file.\n");
        return 0;
    }

    for (int i = 0; i < count; i++)
    {
        fprintf(file, "%d|%s|%.2lf|%d\n",
                accounts[i].accountNo,
                accounts[i].name,
                accounts[i].balance,
                accounts[i].active);
    }

    fclose(file);

    return 1;
}


/* ---------------------------------------------------------
   FIND ACCOUNT
   --------------------------------------------------------- */

int findAccount(Account accounts[], int count, int accountNo)
{
    for (int i = 0; i < count; i++)
    {
        if (accounts[i].accountNo == accountNo &&
            accounts[i].active == 1)
        {
            return i;
        }
    }

    return -1;
}


/* ---------------------------------------------------------
   TRANSACTION LOG
   --------------------------------------------------------- */

void logTransaction(int accountNo,
                    const char *type,
                    double amount,
                    double balance,
                    const char *status)
{
    FILE *file;
    char datetime[50];

    getDateTime(datetime, sizeof(datetime));

    file = fopen("transactions.txt", "a");

    if (file == NULL)
    {
        printf("WARNING: Transaction log could not be written.\n");
        return;
    }

    fprintf(file,
            "%s|Account:%d|Type:%s|Amount:%.2lf|Balance:%.2lf|Status:%s\n",
            datetime,
            accountNo,
            type,
            amount,
            balance,
            status);

    fclose(file);
}


/* ---------------------------------------------------------
   GENERATE ACCOUNT NUMBER
   --------------------------------------------------------- */

int generateAccountNumber(Account accounts[], int count)
{
    int max = 1000;

    for (int i = 0; i < count; i++)
    {
        if (accounts[i].accountNo > max)
            max = accounts[i].accountNo;
    }

    return max + 1;
}


/* ---------------------------------------------------------
   CREATE ACCOUNT
   --------------------------------------------------------- */

void createAccount()
{
    Account accounts[MAX_ACCOUNTS];

    int count = loadAccounts(accounts);

    if (count >= MAX_ACCOUNTS)
    {
        printf("\nBank database is full.\n");
        return;
    }

    Account newAccount;

    newAccount.accountNo =
        generateAccountNumber(accounts, count);

    printf("\nEnter customer name: ");

    clearInputBuffer();

    fgets(newAccount.name,
          MAX_NAME,
          stdin);

    newAccount.name[
        strcspn(newAccount.name, "\n")
    ] = '\0';

    newAccount.balance = 0.0;
    newAccount.active = 1;

    accounts[count] = newAccount;

    if (saveAccounts(accounts, count + 1))
    {
        printf("\n================================\n");
        printf("ACCOUNT CREATED SUCCESSFULLY\n");
        printf("Account Number : %d\n",
               newAccount.accountNo);
        printf("Customer Name  : %s\n",
               newAccount.name);
        printf("Balance        : %.2lf\n",
               newAccount.balance);
        printf("================================\n");

        logTransaction(
            newAccount.accountNo,
            "CREATE_ACCOUNT",
            0,
            newAccount.balance,
            "SUCCESS"
        );
    }
}


/* ---------------------------------------------------------
   DEPOSIT
   --------------------------------------------------------- */

void deposit()
{
    Account accounts[MAX_ACCOUNTS];

    int count = loadAccounts(accounts);

    int accountNo;
    double amount;

    printf("\nEnter account number: ");
    scanf("%d", &accountNo);

    int index =
        findAccount(accounts, count, accountNo);

    if (index == -1)
    {
        printf("\nAccount not found.\n");

        logTransaction(
            accountNo,
            "DEPOSIT",
            0,
            0,
            "FAILED_ACCOUNT_NOT_FOUND"
        );

        return;
    }

    printf("Enter deposit amount: ");
    scanf("%lf", &amount);

    if (amount <= 0)
    {
        printf("\nInvalid amount.\n");

        logTransaction(
            accountNo,
            "DEPOSIT",
            amount,
            accounts[index].balance,
            "FAILED_INVALID_AMOUNT"
        );

        return;
    }

    double oldBalance =
        accounts[index].balance;

    accounts[index].balance += amount;

    if (saveAccounts(accounts, count))
    {
        printf("\nDeposit successful.\n");
        printf("Old Balance : %.2lf\n", oldBalance);
        printf("Deposit     : %.2lf\n", amount);
        printf("New Balance : %.2lf\n",
               accounts[index].balance);

        logTransaction(
            accountNo,
            "DEPOSIT",
            amount,
            accounts[index].balance,
            "SUCCESS"
        );
    }
    else
    {
        /*
           ROLLBACK IN MEMORY
        */

        accounts[index].balance =
            oldBalance;

        printf("\nTransaction failed.\n");
        printf("Rollback performed.\n");

        logTransaction(
            accountNo,
            "DEPOSIT",
            amount,
            oldBalance,
            "FAILED_ROLLBACK"
        );
    }
}


/* ---------------------------------------------------------
   WITHDRAW
   --------------------------------------------------------- */

void withdraw()
{
    Account accounts[MAX_ACCOUNTS];

    int count = loadAccounts(accounts);

    int accountNo;
    double amount;

    printf("\nEnter account number: ");
    scanf("%d", &accountNo);

    int index =
        findAccount(accounts, count, accountNo);

    if (index == -1)
    {
        printf("\nAccount not found.\n");
        return;
    }

    printf("Enter withdrawal amount: ");
    scanf("%lf", &amount);

    if (amount <= 0)
    {
        printf("\nInvalid amount.\n");
        return;
    }

    if (amount > accounts[index].balance)
    {
        printf("\nTransaction FAILED.\n");
        printf("Insufficient balance.\n");

        logTransaction(
            accountNo,
            "WITHDRAW",
            amount,
            accounts[index].balance,
            "FAILED_INSUFFICIENT_BALANCE"
        );

        return;
    }

    double oldBalance =
        accounts[index].balance;

    accounts[index].balance -= amount;

    if (saveAccounts(accounts, count))
    {
        printf("\nWithdrawal successful.\n");
        printf("Withdrawn   : %.2lf\n", amount);
        printf("New Balance : %.2lf\n",
               accounts[index].balance);

        logTransaction(
            accountNo,
            "WITHDRAW",
            amount,
            accounts[index].balance,
            "SUCCESS"
        );
    }
    else
    {
        accounts[index].balance =
            oldBalance;

        printf("\nWithdrawal failed.\n");
        printf("ROLLBACK performed.\n");
    }
}


/* ---------------------------------------------------------
   CHECK BALANCE
   --------------------------------------------------------- */

void checkBalance()
{
    Account accounts[MAX_ACCOUNTS];

    int count = loadAccounts(accounts);

    int accountNo;

    printf("\nEnter account number: ");
    scanf("%d", &accountNo);

    int index =
        findAccount(accounts, count, accountNo);

    if (index == -1)
    {
        printf("\nAccount not found.\n");
        return;
    }

    printf("\n------------------------------\n");
    printf("ACCOUNT DETAILS\n");
    printf("------------------------------\n");
    printf("Account No : %d\n",
           accounts[index].accountNo);
    printf("Name       : %s\n",
           accounts[index].name);
    printf("Balance    : %.2lf\n",
           accounts[index].balance);
    printf("------------------------------\n");
}


/* ---------------------------------------------------------
   TRANSFER MONEY
   --------------------------------------------------------- */

void transferMoney()
{
    Account accounts[MAX_ACCOUNTS];

    int count = loadAccounts(accounts);

    int fromAccount;
    int toAccount;
    double amount;

    printf("\nEnter sender account: ");
    scanf("%d", &fromAccount);

    printf("Enter receiver account: ");
    scanf("%d", &toAccount);

    printf("Enter amount: ");
    scanf("%lf", &amount);

    int sender =
        findAccount(accounts, count, fromAccount);

    int receiver =
        findAccount(accounts, count, toAccount);

    if (sender == -1 || receiver == -1)
    {
        printf("\nTRANSFER FAILED.\n");
        printf("Invalid sender or receiver.\n");
        return;
    }

    if (fromAccount == toAccount)
    {
        printf("\nCannot transfer to same account.\n");
        return;
    }

    if (amount <= 0)
    {
        printf("\nInvalid amount.\n");
        return;
    }

    if (accounts[sender].balance < amount)
    {
        printf("\nTRANSFER FAILED.\n");
        printf("Insufficient sender balance.\n");
        return;
    }

    /*
       Save old values.
       These values are required for rollback.
    */

    double oldSenderBalance =
        accounts[sender].balance;

    double oldReceiverBalance =
        accounts[receiver].balance;

    /*
       STEP 1
       Debit sender
    */

    accounts[sender].balance -= amount;

    /*
       STEP 2
       Credit receiver
    */

    accounts[receiver].balance += amount;

    /*
       STEP 3
       Save complete database
    */

    if (saveAccounts(accounts, count))
    {
        printf("\n================================\n");
        printf("TRANSFER SUCCESSFUL\n");
        printf("From Account : %d\n", fromAccount);
        printf("To Account   : %d\n", toAccount);
        printf("Amount       : %.2lf\n", amount);
        printf("================================\n");

        logTransaction(
            fromAccount,
            "TRANSFER_OUT",
            amount,
            accounts[sender].balance,
            "SUCCESS"
        );

        logTransaction(
            toAccount,
            "TRANSFER_IN",
            amount,
            accounts[receiver].balance,
            "SUCCESS"
        );
    }
    else
    {
        /*
           TRANSACTION FAILED

           ROLLBACK BOTH ACCOUNTS
        */

        accounts[sender].balance =
            oldSenderBalance;

        accounts[receiver].balance =
            oldReceiverBalance;

        printf("\nTRANSFER FAILED.\n");
        printf("ROLLBACK COMPLETED.\n");
    }
}


/* ---------------------------------------------------------
   CLOSE ACCOUNT
   --------------------------------------------------------- */

void closeAccount()
{
    Account accounts[MAX_ACCOUNTS];

    int count = loadAccounts(accounts);

    int accountNo;

    printf("\nEnter account number: ");
    scanf("%d", &accountNo);

    int index =
        findAccount(accounts, count, accountNo);

    if (index == -1)
    {
        printf("\nAccount not found.\n");
        return;
    }

    if (accounts[index].balance != 0)
    {
        printf("\nAccount cannot be closed.\n");
        printf("Balance must be 0.\n");
        return;
    }

    accounts[index].active = 0;

    if (saveAccounts(accounts, count))
    {
        printf("\nAccount closed successfully.\n");

        logTransaction(
            accountNo,
            "CLOSE_ACCOUNT",
            0,
            0,
            "SUCCESS"
        );
    }
}


/* ---------------------------------------------------------
   DISPLAY ALL ACCOUNTS
   --------------------------------------------------------- */

void displayAllAccounts()
{
    Account accounts[MAX_ACCOUNTS];

    int count = loadAccounts(accounts);

    printf("\n");
    printf("=================================================\n");
    printf("                 ALL ACCOUNTS\n");
    printf("=================================================\n");

    printf("%-10s %-25s %-12s %-10s\n",
           "Account",
           "Name",
           "Balance",
           "Status");

    printf("-------------------------------------------------\n");

    for (int i = 0; i < count; i++)
    {
        printf("%-10d %-25s %-12.2lf %-10s\n",
               accounts[i].accountNo,
               accounts[i].name,
               accounts[i].balance,
               accounts[i].active
                   ? "ACTIVE"
                   : "CLOSED");
    }

    printf("=================================================\n");
}


/* ---------------------------------------------------------
   TRANSACTION HISTORY
   --------------------------------------------------------- */

void transactionHistory()
{
    FILE *file;

    file = fopen("transactions.txt", "r");

    if (file == NULL)
    {
        printf("\nNo transaction history found.\n");
        return;
    }

    char line[500];

    printf("\n============================================\n");
    printf("          TRANSACTION HISTORY\n");
    printf("============================================\n");

    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
    }

    fclose(file);
}


/* ---------------------------------------------------------
   MAIN MENU
   --------------------------------------------------------- */

int main()
{
    int choice;

    do
    {
        printf("\n\n");
        printf("============================================\n");
        printf("             MINI BANKING SYSTEM\n");
        printf("============================================\n");

        printf("1. Create Account\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Check Balance\n");
        printf("5. Transfer Money\n");
        printf("6. Close Account\n");
        printf("7. Display All Accounts\n");
        printf("8. Transaction History\n");
        printf("9. Exit\n");

        printf("============================================\n");

        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
            case 1:
                createAccount();
                break;

            case 2:
                deposit();
                break;

            case 3:
                withdraw();
                break;

            case 4:
                checkBalance();
                break;

            case 5:
                transferMoney();
                break;

            case 6:
                closeAccount();
                break;

            case 7:
                displayAllAccounts();
                break;

            case 8:
                transactionHistory();
                break;

            case 9:
                printf("\nThank you for using the banking system.\n");
                break;

            default:
                printf("\nInvalid choice.\n");
        }

    } while (choice != 9);

    return 0;
}
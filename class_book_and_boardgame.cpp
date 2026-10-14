#include<iostream>
#include<string>
#include<windows.h>
using namespace std;

enum ProductStatus{
    AVAILABLE,
    RENTED,
    DAMAGED
};
    
class Product{
    private:
         string productID;
         string name;
         double price;
         int quantity;
         ProductStatus status;
    public:
        Product(string id, string n, double p, int q, ProductStatus s){
            productID = id;
            name = n;
            price = p;
            quantity = q;
            status = s;
        }
    string getProductID() const{
        return productID;
    }
    string getName() const{
        return name;
    }
    double getPrice() const{
        return price;
    }
    int getQuantity() const{
        return quantity;
    }
    ProductStatus getStatus() const{
        return status;
    }

    virtual void display() const{
        cout << " Mã sản phẩm " << productID << endl;
        cout << " Tên sản phẩm " << name << endl;
        cout << " Giá sản phẩm " << price << endl;
        cout << " Số lượng " << quantity << endl;
    }

    virtual double calculatePurchaseFee(int quantity) const = 0;

    virtual double calculateRentalFee(int duration) const = 0;

    virtual ~Product(){}
};

class ComicBook : public Product{
    private:
        string author;
        string genre;
        int volume;
        double rentalPrice;
        double purchasePrice; 
        string publisher;
    public:
        ComicBook(string id, string n, int q, ProductStatus s, string a, string g, int v, string pub, double rent, double buy) : Product(id, n, buy, q, s){
            author = a;
            genre = g;
            volume = v;
            publisher = pub;
            rentalPrice = rent;
            purchasePrice = buy;
        }    
        double calculateRentalFee(int months) const override{
            if (months <= 0)
                return 0;
            return rentalPrice * months;
        }
        double calculatePurchaseFee(int quantity) const override{
            if (quantity <= 0)
                return 0;
            return purchasePrice * quantity;
        }
        void display() const override{
            Product::display();
            cout << " Tác giả " << author << endl;
            cout << " Thể loại " << genre << endl;
            cout << " Tập " << volume << endl;
            cout << " Nhà xuất bản " << publisher << endl;
        }
};

class BoardGame : public Product{
    private:
        string publisher;
        int minPlayers;
        int maxPlayers;
        int playTime;
        int minAge;
        double hourlyRate;

    public:
        BoardGame(string id, string n, int q, ProductStatus s, string pub, int minP, int maxP, int time, int age, double rate) : Product(id, n, rate, q, s){
            publisher = pub;
            minPlayers = minP;
            maxPlayers = maxP;
            playTime = time;
            minAge = age;
            hourlyRate = rate;
        }
        double calculateRentalFee(int hours) const override{
            if (hours <= 0)
                return 0;
            return hourlyRate * hours;
        }
        double calculatePurchaseFee(int) const override{
            return 0;
        }
        void display() const override{
            Product :: display();
            cout << " Nhà xuất bản " << publisher << endl;
            cout << "So nguoi choi: " << minPlayers << " - " << maxPlayers << endl;
            cout << " Thời gian chơi " << playTime << " phút " << endl;
            cout << " Tuổi tối thiểu " << minAge << endl;
            cout << " Giá thuê theo giờ " << hourlyRate << endl;
        }
};

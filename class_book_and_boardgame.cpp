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
        ComicBook(string id, string n, double p, int q, ProductStatus s, string a, string g, int v, string pub, double rent, double buy) : Product(id, n, buy, q, s){
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
            return getPrice * months;
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
    public:
        BoardGame(string id, string n, double p, int q, ProductStatus s, string pub, int minP, int maxP, int time, int age) : Product(id, n, p, q, s){
            publisher = pub;
            minPlayers = minP;
            maxPlayers = maxP;
            playTime = time;
            minAge = age;
        }
        double calculateRentalFee(int hours) const override{
            if (hours <= 0)
                return 0;
            return getPrice() * hours;
        }
        void display() const override{
            Product :: display();
            cout << " Nhà xuất bản " << publisher << endl;
            cout << "So nguoi choi: " << minPlayers << " - " << maxPlayers << endl;
            cout << " Thời gian chơi " << playTime << " phút " << endl;
            cout << " Tuổi tối thiểu " << minAge << endl;
        }
};

int main(){
    ComicBook comic("CB01", "Doraemon", 3000, 10, AVAILABLE, "Fujiko F. Fujio", "Hài hước", 1, " Kim Đồng");
    BoardGame game("BG01", "Catan", 30000, 5, AVAILABLE, "Catan Studio", 3, 4, 60, 10);
    
    cout << " ======= TRUYỆN TRANH ======= " << endl;
    comic.display();
    
    cout << " Phí thuê 3 ngày " << comic.calculateRentalFee(3) << endl;

    cout << " \n======= BOARD GAME ======= " << endl;
    game.display();

    cout << " Phí thuê 2 giờ " << game.calculateRentalFee(2) << endl;

    return 0;
}


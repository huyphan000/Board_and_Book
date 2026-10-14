#include<iostream>
#include<string>
#include<windows.h>
using namespace std;

// Khai báo các trạng thái của sản phẩm
enum ProductStatus{
    AVAILABLE, // Sản phẩm có sẵn
    RENTED, // Sản phẩm đang được thuê
    DAMAGED // Sản phẩm bị hư hỏng
};

// Lớp cha Product, đại diện cho sản phẩm
class Product{
    private:
         string productID; // Mã sản phẩm
         string name; // Tên sản phẩm
         double price; // Giá sản phẩm
         int quantity; // Số lượng sản phẩm
         ProductStatus status; // Trạng thái sản phẩm
    public:
        // Hàm khởi tạo, gán giá trị ban đầu cho các thuộc tính
        Product(string id, string n, double p, int q, ProductStatus s){
            productID = id; // Gán mã sản phẩm
            name = n; // Gán tên sản phẩm
            price = p; // Gán giá sản phẩm
            quantity = q; // Gán số lượng sản phẩm
            status = s; // Gán trạng thái sản phẩm
        }
    // Hàm trả về mã sản phẩm
    string getProductID() const{
        return productID;
    }
    // Hàm trả về tên sản phẩm
    string getName() const{
        return name;
    }
    // Hàm trả về giá sản phẩm 
    double getPrice() const{
        return price;
    }
    // Hàm trả về số lượng sản phẩm
    int getQuantity() const{
        return quantity;
    }
    // Hàm trả về trạng thái sản phẩm
    ProductStatus getStatus() const{
        return status;
    }
    
    // Hàm ảo hiện thị thông tin sản phẩm
    virtual void display() const{
        cout << " Mã sản phẩm " << productID << endl; // In mã sản phẩm
        cout << " Tên sản phẩm " << name << endl; // In tên sản phẩm
        cout << " Giá sản phẩm " << price << " VND " << endl; // In giá sản phẩm
        cout << " Số lượng " << quantity << endl; // In số lượng 
    }
    
    // Hàm thuần ảo tính phí mua sản phẩm
    virtual double calculatePurchaseFee(int quantity) const = 0;
    
    // Hàm thuần ảo tính phí thuê sản phẩm
    virtual double calculateRentalFee(int duration) const = 0;

    virtual ~Product(){} // Hàm hủy lớp cha khi đối tượng được hủy 
};

// Lớp ComicBook kế thừa từ lớp Product, đại diện cho truyện tranh
class ComicBook : public Product{
    private:
        string author; // Tên tác giả
        string genre; // Thể loại truyện
        int volume; // Số tập truyện
        double rentalPrice; // Giá thuê một tháng
        double purchasePrice; // Giá mua một cuốn
        string publisher; // Nhà xuất bản
    public:
        // Hàm khởi tạo cho truyện tranh 
        ComicBook(string id, string n, int q, ProductStatus s, string a, string g, int v, string pub, double rent, double buy) : Product(id, n, buy, q, s){ // Hàm khởi tạo của lớp cha
            author = a; // Gán tác giả
            genre = g; // Gán thể loại
            volume = v; // Gán số tập
            publisher = pub; // Gán nhà xuất bản
            rentalPrice = rent; // Gán giá thuê
            purchasePrice = buy; // Gán giá mua 
        }    

        // Hàm tính phí thuê truyện theo 1 tháng
        double calculateRentalFee(int months) const override{
            if (months <= 0)
                return 0;
            return rentalPrice * months;
        }

        // Hàm tính phí mua truyện theo số lượng
        double calculatePurchaseFee(int quantity) const override{
            if (quantity <= 0)
                return 0;
            return purchasePrice * quantity;
        }

        // Ghi đè hàm hiển thị thông tin của lớp cha
        void display() const override{
            Product::display(); // Hiển thị thông tin chung của sản phẩm 
            cout << " Tác giả " << author << endl; // In tác giả
            cout << " Thể loại " << genre << endl; // In thể loại
            cout << " Tập " << volume << " tập " << endl; // In số tập 
            cout << " Nhà xuất bản " << publisher << endl; // In nhà xuất bản
            cout << " Giá thuê : " << rentalPrice << " VND " << endl; // In giá thuê mỗi tháng
            cout << " Giá mua : " << purchasePrice << " VND " << endl; // In giá mua mỗi cuốn
        }
};

// Lớp BoardGame kế thừa từ lớp Product, đại diện cho trò chơi bàn 
class BoardGame : public Product{
    private:
        string publisher; // Nhà xuất bản 
        int minPlayers; // Số người chơi tối thiểu
        int maxPlayers; // Số người chơi tối đa
        int playTime; // Thời gian chơi tính bằng giờ
        int minAge; // Độ tuổi tối thiểu
        double hourlyRate; // Giá thuê một giờ chơi

    public:
        // Hàm khởi tạo cho BoardGame
        BoardGame(string id, string n, int q, ProductStatus s, string pub, int minP, int maxP, int time, int age, double rate) : Product(id, n, rate, q, s){
            publisher = pub; // Gán nhà xuất bản
            minPlayers = minP; // Gán số người chơi tối thiểu
            maxPlayers = maxP; // Gán số người chơi tối đa
            playTime = time; // Gán số giờ chơi
            minAge = age; // Gán độ tuổi tối thiểu
            hourlyRate = rate; // Gán giá thuê theo giờ
        }
        // Hàm tính phí thuê BoardGame theo giờ 
        double calculateRentalFee(int hours) const override{
            if (hours <= 0)
                return 0;
            return hourlyRate * hours;
        }
        // BoardGame không bán nên phí mua trả về 0
        double calculatePurchaseFee(int) const override{
            return 0;
        }
        // Ghi đè hàm hiển thị thông tin BoardGame 
        void display() const override{
            Product :: display();
            cout << " Nhà xuất bản " << publisher << endl; // In nhà xuất bản 
            cout << "So nguoi choi: " << minPlayers << " - " << maxPlayers << endl; // In số người chơi
            cout << " Thời gian chơi " << playTime << " giờ " << endl; // In thời gian chơi
            cout << " Tuổi tối thiểu " << minAge << endl; // In độ tuổi tối thiểu
            cout << " Giá thuê theo giờ " << hourlyRate << " VND " <<endl; // In giá thuê theo giờ
        }
};

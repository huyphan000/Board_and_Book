#ifndef PRODUCT_SYSTEM_CPP
#define PRODUCT_SYSTEM_CPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

// =====================================================================
// HE THONG SAN PHAM (PRODUCT) & QUAN LY TON KHO (INVENTORY)
// Du an: Board and Book (C++ OOP) - Thanh vien 1: Pham Quang Sang
// Gom tron bo 9 lop:
//   - Giao dien (Interface): IProductInfo
//   - Tang 1 (Lop cha truu tuong): Product (+ enum ProductStatus)
//   - Tang 2 (Lop trung gian): BookProduct, GameProduct
//   - Tang 3 (Lop cu the): ComicBook, MangaSet, BoardGame, CardGame
//   - Quan ly ton kho: InventoryManager
// =====================================================================


// =====================================================================
// GIAO DIEN (INTERFACE): IProductInfo
// Chuan hoa cac phuong thuc hien thi, phan loai va tom tat thong tin san pham
// Khong chua thuoc tinh, chi chua cac ham thuan ao
// =====================================================================
class IProductInfo {
public:
    virtual void display() const = 0;          // Hien thi thong tin chi tiet
    virtual string getProductType() const = 0; // Lay ten loai san pham cu the
    virtual string getSummaryInfo() const = 0; // Lay chuoi tom tat ngan gon
    virtual ~IProductInfo() {}                 // Ham huy ao cho Interface
};


// Trang thai san pham (khong dung RENTED cho toan bo mat hang co nhieu ban)
enum ProductStatus {
    AVAILABLE,    // Dang kinh doanh / con hang trong kho
    OUT_OF_STOCK, // Tam het hang trong kho (quantity == 0)
    DAMAGED       // San pham bi hu hong / ngung phuc vu
};

// =====================================================================
// TANG 1 - LOP CHA TRUU TUONG: Product (Ke thua giao dien IProductInfo)
// =====================================================================
class Product : public IProductInfo {
private:
    string productID;      // Ma san pham (dong goi kin)
    ProductStatus status;  // Trang thai tong quat cua san pham

protected:
    string name;             // Ten san pham
    double basePrice;        // Gia co so (VND)
    int quantity;            // So luong ton kho kha dung
    double lateFeePerDay;    // Phi tre han moi ngay (VND/ngay)
    double replacementCost;  // Chi phi den bu neu mat / hong nang (VND)

public:
    // Ham khoi tao co kiem tra dieu kien dau vao (gia >= 0, so luong >= 0)
    Product(string id, string n, double price, int qty, ProductStatus s = AVAILABLE,
            double lateFee = 0.0, double replaceCost = -1.0) {
        productID = id;
        name = n;
        basePrice = (price >= 0.0) ? price : 0.0;
        quantity = (qty >= 0) ? qty : 0;
        lateFeePerDay = (lateFee >= 0.0) ? lateFee : 0.0;
        replacementCost = (replaceCost >= 0.0) ? replaceCost : basePrice;

        if (s == DAMAGED) {
            status = DAMAGED;
        } else {
            status = (quantity > 0) ? AVAILABLE : OUT_OF_STOCK;
        }
    }

    // Cac ham truy van (Getter)
    string getProductID() const { return productID; }
    string getName() const { return name; }
    double getPrice() const { return basePrice; }
    double getBasePrice() const { return basePrice; }
    int getQuantity() const { return quantity; }
    ProductStatus getStatus() const { return status; }
    double getLateFeePerDay() const { return lateFeePerDay; }
    double getReplacementCost() const { return replacementCost; }

    // Cac ham cap nhat co kiem tra hop le (tra ve bool)
    bool setPrice(double price) {
        if (price < 0.0) return false;
        basePrice = price;
        return true;
    }

    bool setLateFeePerDay(double fee) {
        if (fee < 0.0) return false;
        lateFeePerDay = fee;
        return true;
    }

    bool setReplacementCost(double cost) {
        if (cost < 0.0) return false;
        replacementCost = cost;
        return true;
    }

    bool setStatus(ProductStatus s) {
        if (s == AVAILABLE && quantity <= 0) {
            status = OUT_OF_STOCK;
            return false;
        }
        status = s;
        return true;
    }

    // Kiem tra kha dung theo so luong yeu cau
    bool isAvailable(int requestedQty = 1) const {
        if (requestedQty <= 0) return false;
        return (status != DAMAGED) && (quantity >= requestedQty);
    }

    // Tang ton kho (nhap hang / khach tra hang / giai phong giu cho)
    bool increaseStock(int amount) {
        if (amount <= 0) return false;
        quantity += amount;
        if (status == OUT_OF_STOCK) {
            status = AVAILABLE;
        }
        return true;
    }

    // Giam ton kho (khach thue / mua / giu cho)
    bool decreaseStock(int amount) {
        if (amount <= 0 || status == DAMAGED || quantity < amount) {
            return false;
        }
        quantity -= amount;
        if (quantity == 0) {
            status = OUT_OF_STOCK;
        }
        return true;
    }

    // Chuyen trang thai sang chuoi de hien thi
    string getStatusText() const {
        if (status == AVAILABLE) return "AVAILABLE";
        if (status == OUT_OF_STOCK) return "OUT_OF_STOCK";
        return "DAMAGED";
    }

    // Cai dat phuong thuc tu giao dien IProductInfo
    void display() const override {
        cout << " Loai san pham: " << getProductType() << endl;
        cout << " Ma san pham: " << productID << " | Ten: " << name << endl;
        cout << " Gia co so: " << basePrice << " VND | So luong ton: " << quantity
             << " | Trang thai: " << getStatusText() << endl;
        cout << " Phi tre han/ngay: " << lateFeePerDay
             << " VND | Chi phi den bu: " << replacementCost << " VND" << endl;
    }

    string getSummaryInfo() const override {
        return "[" + productID + "] " + name + " (" + getProductType() + " - " + getStatusText() + ")";
    }

    // Cac ham thuan ao bat buoc lop con/chau ghi de (Truu tuong + Da hinh)
    string getProductType() const override = 0;
    virtual double calculateRentalFee(int duration) const = 0;
    virtual double calculatePurchaseFee(int qty) const = 0;

    // Ham huy ao de giai phong dung doi tuong lop con qua con tro Product* / IProductInfo*
    virtual ~Product() {}
};


// =====================================================================
// TANG 2 - LOP CON TRUNG GIAN: BookProduct (Ke thua tu Product)
// Gom cac thuoc tinh va quy tac chung cua nhom sach / truyen tranh
// =====================================================================
class BookProduct : public Product {
protected:
    string author;    // Tac gia
    string publisher; // Nha xuat ban
    string genre;     // The loai

public:
    BookProduct(string id, string n, double price, int qty, ProductStatus s,
                string a, string pub, string g,
                double lateFee = 0.0, double replaceCost = -1.0)
        : Product(id, n, price, qty, s, lateFee, replaceCost) {
        author = a;
        publisher = pub;
        genre = g;
    }

    string getAuthor() const { return author; }
    string getPublisher() const { return publisher; }
    string getGenre() const { return genre; }

    // Kiem tra thong tin sach/truyen co day du hay khong
    bool validateBookInfo() const {
        return !getProductID().empty() &&
               !name.empty() &&
               !author.empty() &&
               !publisher.empty() &&
               !genre.empty();
    }

    void display() const override {
        Product::display();
        cout << " Tac gia: " << author << " | NXB: " << publisher << endl;
        cout << " The loai: " << genre << endl;
    }

    virtual ~BookProduct() {}
};


// =====================================================================
// TANG 2 - LOP CON TRUNG GIAN: GameProduct (Ke thua tu Product)
// Gom cac thuoc tinh va quy tac chung cua nhom tro choi (BoardGame / CardGame)
// =====================================================================
class GameProduct : public Product {
protected:
    string publisher; // Nha xuat ban
    int minPlayers;   // So nguoi choi toi thieu
    int maxPlayers;   // So nguoi choi toi da
    int playTime;     // Thoi gian choi (gio)
    int minAge;       // Do tuoi toi thieu

public:
    GameProduct(string id, string n, double price, int qty, ProductStatus s,
                string pub, int minP, int maxP, int timeH, int age,
                double lateFee = 0.0, double replaceCost = -1.0)
        : Product(id, n, price, qty, s, lateFee, replaceCost) {
        publisher = pub;
        minPlayers = (minP >= 1) ? minP : 1;
        maxPlayers = (maxP >= minPlayers) ? maxP : minPlayers;
        playTime = (timeH > 0) ? timeH : 1;
        minAge = (age >= 0) ? age : 0;
    }

    string getPublisher() const { return publisher; }
    int getMinPlayers() const { return minPlayers; }
    int getMaxPlayers() const { return maxPlayers; }
    int getPlayTime() const { return playTime; }
    int getMinAge() const { return minAge; }

    // Kiem tra so nguoi choi co nam trong gioi han cho phep hay khong
    virtual bool checkPlayerLimit(int playerCount) const {
        return playerCount >= minPlayers && playerCount <= maxPlayers;
    }

    // Kiem tra do tuoi nguoi choi co phu hop hay khong
    bool isAgeSuitable(int age) const {
        return age >= minAge;
    }

    void display() const override {
        Product::display();
        cout << " Nha xuat ban: " << publisher << endl;
        cout << " So nguoi choi: " << minPlayers << " - " << maxPlayers
             << " | Thoi gian choi: " << playTime << " gio | Tuoi toi thieu: " << minAge << "+" << endl;
    }

    virtual ~GameProduct() {}
};


// =====================================================================
// TANG 3 - LOP CHAU: ComicBook (Ke thua tu BookProduct)
// Dai dien cho truyen tranh ban/thue theo tung tap le
// =====================================================================
class ComicBook : public BookProduct {
private:
    int volume;           // So tap truyen
    double rentalPrice;   // Gia thue theo thang (VND)
    double purchasePrice; // Gia mua 1 cuon (VND)

public:
    ComicBook(string id, string n, int qty, ProductStatus s,
              string a, string pub, string g,
              int vol, double rentPrice, double buyPrice,
              double lateFee = 0.0, double replaceCost = -1.0)
        : BookProduct(id, n, buyPrice, qty, s, a, pub, g, lateFee, replaceCost) {
        volume = (vol > 0) ? vol : 1;
        rentalPrice = (rentPrice >= 0.0) ? rentPrice : 0.0;
        purchasePrice = (buyPrice >= 0.0) ? buyPrice : 0.0;
    }

    int getVolume() const { return volume; }
    double getRentalPrice() const { return rentalPrice; }
    double getPurchasePrice() const { return purchasePrice; }

    string getProductType() const override {
        return "ComicBook";
    }

    // Tinh phi thue truyen le theo so thang
    double calculateRentalFee(int months) const override {
        if (months <= 0) return 0.0;
        return rentalPrice * months;
    }

    // Tinh phi mua truyen le theo so luong
    double calculatePurchaseFee(int qty) const override {
        if (qty <= 0) return 0.0;
        return purchasePrice * qty;
    }

    void display() const override {
        BookProduct::display();
        cout << " So tap: Tap " << volume << endl;
        cout << " Gia thue/thang: " << rentalPrice << " VND | Gia mua/cuon: " << purchasePrice << " VND" << endl;
    }
};


// =====================================================================
// TANG 3 - LOP CHAU: MangaSet (Ke thua tu BookProduct)
// Dai dien cho tron bo truyen tranh (co chinh sach giam gia theo bo)
// =====================================================================
class MangaSet : public BookProduct {
private:
    int totalVolumes;      // Tong so tap trong bo
    double bundleDiscount; // Ty le giam gia theo bo (0.0 -> 0.9, vi du 0.15 = giam 15%)
    double rentalPrice;    // Gia thue ca bo theo thang truoc khi giam (VND)

public:
    // totalBasePrice la tong gia goc chua giam cua ca bo truyen
    MangaSet(string id, string n, double totalBasePrice, int qty, ProductStatus s,
             string a, string pub, string g,
             int totalVols, double discountRate, double rentPrice,
             double lateFee = 0.0, double replaceCost = -1.0)
        : BookProduct(id, n, totalBasePrice, qty, s, a, pub, g, lateFee, replaceCost) {
        totalVolumes = (totalVols > 0) ? totalVols : 1;
        if (discountRate < 0.0) bundleDiscount = 0.0;
        else if (discountRate > 0.9) bundleDiscount = 0.9;
        else bundleDiscount = discountRate;
        rentalPrice = (rentPrice >= 0.0) ? rentPrice : 0.0;
    }

    int getTotalVolumes() const { return totalVolumes; }
    double getBundleDiscount() const { return bundleDiscount; }
    double getRentalPrice() const { return rentalPrice; }

    string getProductType() const override {
        return "MangaSet";
    }

    // Tinh so tien duoc giam khi mua 1 bo
    double calculateBundleDiscount() const {
        return basePrice * bundleDiscount;
    }

    // Tinh gia thuc te sau khi giam cho 1 bo
    double calculateBundlePrice() const {
        return basePrice - calculateBundleDiscount();
    }

    // Tinh phi mua theo so luong bo
    double calculatePurchaseFee(int qty) const override {
        if (qty <= 0) return 0.0;
        return calculateBundlePrice() * qty;
    }

    // Tinh phi thue ca bo theo so thang (ap dung uu dai giam gia bo)
    double calculateRentalFee(int months) const override {
        if (months <= 0) return 0.0;
        return rentalPrice * months * (1.0 - bundleDiscount);
    }

    void display() const override {
        BookProduct::display();
        cout << " Tong so tap: " << totalVolumes << " tap | Giam gia bo: "
             << (bundleDiscount * 100.0) << "%" << endl;
        cout << " Gia tron bo sau giam: " << calculateBundlePrice()
             << " VND | Gia thue bo/thang: " << calculateRentalFee(1) << " VND" << endl;
    }
};


// =====================================================================
// TANG 3 - LOP CHAU: BoardGame (Ke thua tu GameProduct)
// Dai dien cho tro choi ban co / ban choi (chi cho thue theo gio)
// =====================================================================
class BoardGame : public GameProduct {
private:
    double hourlyRate; // Gia thue theo gio (VND)

public:
    BoardGame(string id, string n, int qty, ProductStatus s,
              string pub, int minP, int maxP, int timeH, int age,
              double rate, double lateFee = 0.0, double replaceCost = -1.0)
        : GameProduct(id, n, rate, qty, s, pub, minP, maxP, timeH, age, lateFee, replaceCost) {
        hourlyRate = (rate >= 0.0) ? rate : 0.0;
    }

    double getHourlyRate() const { return hourlyRate; }

    string getProductType() const override {
        return "BoardGame";
    }

    // Tinh phi thue BoardGame theo so gio
    double calculateRentalFee(int hours) const override {
        if (hours <= 0) return 0.0;
        return hourlyRate * hours;
    }

    // BoardGame khong ban dut nen phi mua tra ve 0
    double calculatePurchaseFee(int) const override {
        return 0.0;
    }

    // Ghi de kiem tra gioi han nguoi choi tu GameProduct
    bool checkPlayerLimit(int playerCount) const override {
        return GameProduct::checkPlayerLimit(playerCount);
    }

    void display() const override {
        GameProduct::display();
        cout << " Gia thue theo gio: " << hourlyRate << " VND/gio (Chi cho thue)" << endl;
    }
};


// =====================================================================
// TANG 3 - LOP CHAU: CardGame (Ke thua tu GameProduct)
// Dai dien cho tro choi the bai (UNO, Exploding Kittens, Dominion...)
// =====================================================================
class CardGame : public GameProduct {
private:
    int cardCount;       // So luong la bai trong bo
    string cardCategory; // Phan loai the bai (The bai gia dinh, Xay dung bo bai...)
    double hourlyRate;   // Gia thue theo gio (VND)

public:
    CardGame(string id, string n, int qty, ProductStatus s,
             string pub, int minP, int maxP, int timeH, int age,
             int cards, string category, double rate,
             double lateFee = 0.0, double replaceCost = -1.0)
        : GameProduct(id, n, rate, qty, s, pub, minP, maxP, timeH, age, lateFee, replaceCost) {
        cardCount = (cards > 0) ? cards : 52;
        cardCategory = category;
        hourlyRate = (rate >= 0.0) ? rate : 0.0;
    }

    int getCardCount() const { return cardCount; }
    string getCardCategory() const { return cardCategory; }
    double getHourlyRate() const { return hourlyRate; }

    string getProductType() const override {
        return "CardGame";
    }

    // Tinh phi thue CardGame theo gio (thue tu 3 gio tro len giam 10%)
    double calculateRentalFee(int hours) const override {
        if (hours <= 0) return 0.0;
        double total = hourlyRate * hours;
        return (hours >= 3) ? total * 0.90 : total;
    }

    // CardGame chi cho thue tai cua hang
    double calculatePurchaseFee(int) const override {
        return 0.0;
    }

    void display() const override {
        GameProduct::display();
        cout << " Loai the bai: " << cardCategory << " | So la bai: " << cardCount << " la" << endl;
        cout << " Gia thue theo gio: " << hourlyRate << " VND/gio (Giam 10% khi thue >= 3 gio)" << endl;
    }
};


// =====================================================================
// LOP QUAN LY TON KHO: InventoryManager (Dung std::map de tra cuu nhanh)
// Quan he su dung / dieu phoi ton kho - khong ke thua Product
// =====================================================================
class InventoryManager {
private:
    map<string, Product*> stockMap;    // Tra cuu nhanh san pham theo productID
    map<string, int> reservedQuantity; // Theo doi so luong dang duoc giu cho

public:
    // Dang ky 1 san pham vao kho de quan ly
    bool registerProduct(Product* product) {
        if (product == nullptr || product->getProductID().empty()) {
            return false;
        }
        stockMap[product->getProductID()] = product;
        if (reservedQuantity.find(product->getProductID()) == reservedQuantity.end()) {
            reservedQuantity[product->getProductID()] = 0;
        }
        return true;
    }

    // Dang ky danh sach san pham vao kho
    void registerProducts(const vector<Product*>& danhSach) {
        for (Product* p : danhSach) {
            registerProduct(p);
        }
    }

    // Tim san pham theo ma (O(log n) nho std::map)
    Product* findProduct(const string& productID) const {
        auto it = stockMap.find(productID);
        if (it == stockMap.end()) return nullptr;
        return it->second;
    }

    // Kiem tra ton kho co du so luong yeu cau hay khong
    bool checkStock(const string& productID, int requestedQty = 1) const {
        Product* p = findProduct(productID);
        if (p == nullptr) return false;
        return p->isAvailable(requestedQty);
    }

    // Giu cho ton kho truoc khi hoan tat giao dich
    bool reserveStock(const string& productID, int qty) {
        Product* p = findProduct(productID);
        if (p == nullptr || qty <= 0) return false;
        if (!p->decreaseStock(qty)) return false;
        reservedQuantity[productID] += qty;
        return true;
    }

    // Giai phong so luong da giu cho (khi huy giao dich)
    bool releaseStock(const string& productID, int qty) {
        Product* p = findProduct(productID);
        if (p == nullptr || qty <= 0) return false;
        if (reservedQuantity[productID] < qty) return false;
        if (!p->increaseStock(qty)) return false;
        reservedQuantity[productID] -= qty;
        return true;
    }

    // Tang ton kho (nhap hang / khach tra hang)
    bool increaseStock(const string& productID, int qty) {
        Product* p = findProduct(productID);
        if (p == nullptr) return false;
        return p->increaseStock(qty);
    }

    // Giam ton kho truc tiep (ban / thue)
    bool decreaseStock(const string& productID, int qty) {
        Product* p = findProduct(productID);
        if (p == nullptr) return false;
        return p->decreaseStock(qty);
    }

    // Dem so luong mat hang theo loai nho ham ao getProductType()
    int countByType(const string& productType) const {
        int count = 0;
        for (const auto& entry : stockMap) {
            if (entry.second != nullptr && entry.second->getProductType() == productType) {
                count++;
            }
        }
        return count;
    }

    int getReservedCount(const string& productID) const {
        auto it = reservedQuantity.find(productID);
        if (it == reservedQuantity.end()) return 0;
        return it->second;
    }

    size_t getTotalTrackedProducts() const {
        return stockMap.size();
    }
};

#endif

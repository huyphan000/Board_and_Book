
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <algorithm>
#include "class_payment_FeeCalculator.cpp" // đã gồm sẵn 2 file class còn lại
#include "data_structures.cpp"
using namespace std;

// ============================================================
// CAC HAM KIEM TRA VA XU LY DU LIEU DAU VAO
// ============================================================

bool laNamNhuan(int nam) {
    return (nam % 400 == 0) || (nam % 4 == 0 && nam % 100 != 0);
}

int soNgayTrongThang(int thang, int nam) {
    static const int soNgay[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    if (thang < 1 || thang > 12) return 0;
    if (thang == 2 && laNamNhuan(nam)) return 29;
    return soNgay[thang - 1];
}

bool ngayHopLe(int ngay, int thang, int nam) {
    if (nam < 1970 || nam > 9999 || thang < 1 || thang > 12) return false;
    return ngay >= 1 && ngay <= soNgayTrongThang(thang, nam);
}

// Doc ngay dd/mm/yyyy hoac dd-mm-yyyy; khong chap nhan ky tu thua.
bool docNgay(const string& chuoi, int& ngay, int& thang, int& nam) {
    istringstream input(chuoi);
    char dau1 = '\0', dau2 = '\0';
    if (!(input >> ngay >> dau1 >> thang >> dau2 >> nam)) return false;
    input >> ws;
    if (!input.eof()) return false;
    if ((dau1 != '/' && dau1 != '-') || dau2 != dau1) return false;
    return ngayHopLe(ngay, thang, nam);
}

// So ngay tu mot moc co dinh theo lich Gregorian; khong phu thuoc gio he thong/DST.
long long ngayThanhSo(int nam, int thang, int ngay) {
    nam -= (thang <= 2);
    const int era = (nam >= 0 ? nam : nam - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(nam - era * 400);
    const int thangDaDoi = thang + (thang > 2 ? -3 : 9);
    const unsigned doy = static_cast<unsigned>((153 * thangDaDoi + 2) / 5 + ngay - 1);
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<long long>(era) * 146097 + doe;
}

Date congThang(const Date& ngayBatDau, int soThang) {
    int tongThang = ngayBatDau.getYear() * 12 + (ngayBatDau.getMonth() - 1) + soThang;
    int namMoi = tongThang / 12;
    int thangMoi = tongThang % 12 + 1;
    int ngayMoi = ngayBatDau.getDay();
    int ngayToiDa = soNgayTrongThang(thangMoi, namMoi);
    if (ngayMoi > ngayToiDa) ngayMoi = ngayToiDa;
    return Date(ngayMoi, thangMoi, namMoi);
}

bool docSoNguyen(const string& chuoi, int& giaTri) {
    istringstream input(chuoi);
    input >> ws;
    if (!(input >> giaTri)) return false;
    input >> ws;
    return input.eof();
}

bool docCoKhong(const string& chuoi, bool macDinh = true) {
    if (chuoi.empty()) return macDinh;
    string giaTri = chuoi;
    transform(giaTri.begin(), giaTri.end(), giaTri.begin(),
              [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return giaTri == "yes" || giaTri == "y" || giaTri == "true" || giaTri == "1";
}

int timViTriSanPham(const vector<Product*>& dsSanPham, const string& maSanPham) {
    for (size_t i = 0; i < dsSanPham.size(); ++i) {
        if (dsSanPham[i] != nullptr && dsSanPham[i]->getProductID() == maSanPham) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int timViTriPhieu(const vector<Rental*>& dsPhieu, const string& maPhieu) {
    for (size_t i = 0; i < dsPhieu.size(); ++i) {
        if (dsPhieu[i] != nullptr && dsPhieu[i]->getRentalId() == maPhieu) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int timViTriKhachHang(const vector<Customer*>& dsKhachHang, const string& maKhachHang) {
    for (size_t i = 0; i < dsKhachHang.size(); ++i) {
        if (dsKhachHang[i] != nullptr && dsKhachHang[i]->getCustomerId() == maKhachHang) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int timViTriThanhToan(const vector<Payment*>& dsThanhToan, const string& maThanhToan) {
    for (size_t i = 0; i < dsThanhToan.size(); ++i) {
        if (dsThanhToan[i] != nullptr && dsThanhToan[i]->getPaymentId() == maThanhToan) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int main() {
    SetConsoleOutputCP(CP_UTF8); // hiển thị được tiếng Việt trên console
    SetConsoleCP(CP_UTF8);       // nhập được tiếng Việt trên console
    // Các class in số tiền bằng cout << double. Nếu không đặt fixed thì số từ
    // 1.000.000 trở lên bị in dạng khoa học (VD: 1e+06)
    cout << fixed << setprecision(0);

    // ===================== 1. KHAI BÁO DỮ LIỆU =====================
    vector<Product*> dsSanPham;    // danh sách sản phẩm (ComicBook và BoardGame)
    // 3 danh sách dưới đây lưu mức phí riêng của từng sản phẩm đọc từ file CSV
    // (các class sản phẩm chưa có thuộc tính này), cùng vị trí với dsSanPham
    vector<double> dsPhiTre;       // phí trễ hạn mỗi ngày
    vector<double> dsPhiHuNhe;     // phí hư hỏng nhẹ
    vector<double> dsPhiHuNang;    // phí hư hỏng nặng
    vector<bool> dsChoThue;        // sản phẩm có được phép cho thuê theo CSV hay không
    vector<bool> dsChoBan;         // sản phẩm có được phép bán theo CSV hay không
    vector<Customer*> dsKhachHang; // danh sách khách hàng (thường và VIP)
    vector<Rental*> dsPhieu;       // danh sách phiếu thuê / mua
    vector<double> dsSoTien;       // số tiền cần thanh toán của từng phiếu, cùng vị trí với dsPhieu
    vector<Payment*> dsThanhToan;  // danh sách các lần thanh toán
    FeeCalculator tinhPhi;         // đối tượng tính phí
    string s, dong;                // s: chuỗi nhập từ bàn phím, dong: một dòng đọc từ file
    char ma[20];                   // dùng để tạo mã tự động: KH001, GD001, TT001...

    // ===================== 2. ĐỌC SẢN PHẨM TỪ FILE CSV =====================
    // f = 0: file truyện tranh, f = 1: file board game
    for (int f = 0; f < 2; f++) {
        string tenFile = (f == 0) ? "comic_books.csv" : "board_games.csv";
        ifstream file(tenFile.c_str()); // mở file
        if (!file.is_open()) {
            cout << "Không mở được file " << tenFile << " (phải để cùng thư mục với chương trình)!" << endl;
            continue;
        }
        getline(file, dong); // dòng đầu tiên chứa tên các cột
        // File CSV lưu dạng UTF-8 có BOM (3 byte đầu EF BB BF), phải bỏ đi,
        // nếu không sẽ không tìm thấy cột "product_id"
        if (dong.size() >= 3 && (unsigned char)dong[0] == 0xEF && (unsigned char)dong[1] == 0xBB && (unsigned char)dong[2] == 0xBF)
            dong.erase(0, 3);
        vector<string> tenCot = tachDongCSV(dong);

        while (getline(file, dong)) { // đọc từng dòng dữ liệu
            if (!dong.empty() && dong[dong.size() - 1] == '\r')
                dong.erase(dong.size() - 1); // bỏ ký tự xuống dòng kiểu Windows
            if (dong.empty())
                continue;
            vector<string> cot = tachDongCSV(dong);
            // Các cột dùng chung của cả 2 loại sản phẩm
            string id = layGiaTri(cot, tenCot, "product_id");
            string ten = layGiaTri(cot, tenCot, "display_name");
            if (id.empty() || ten.empty()) {
                cout << "Bo qua dong CSV khong co ma hoac ten san pham trong " << tenFile << endl;
                continue;
            }
            int soLuong = docSo(layGiaTri(cot, tenCot, "quantity"));
            if (soLuong < 0) soLuong = 0;
            ProductStatus trangThai = docTrangThai(layGiaTri(cot, tenCot, "status"));
            string nxb = layGiaTri(cot, tenCot, "publisher");
            double giaThue = docSo(layGiaTri(cot, tenCot, "rental_price_vnd"));
            if (f == 0) // truyện tranh -> tạo đối tượng ComicBook
                dsSanPham.push_back(new ComicBook(id, ten, soLuong, trangThai,
                    layGiaTri(cot, tenCot, "author"), layGiaTri(cot, tenCot, "genre"),
                    docSo(layGiaTri(cot, tenCot, "volume")), nxb, giaThue,
                    docSo(layGiaTri(cot, tenCot, "purchase_price_vnd"))));
            else        // board game -> tạo đối tượng BoardGame
                dsSanPham.push_back(new BoardGame(id, ten, soLuong, trangThai, nxb,
                    docSo(layGiaTri(cot, tenCot, "min_players")), docSo(layGiaTri(cot, tenCot, "max_players")),
                    docSo(layGiaTri(cot, tenCot, "play_time_hours")), docSo(layGiaTri(cot, tenCot, "min_age")),
                    giaThue));
            dsPhiTre.push_back(docSo(layGiaTri(cot, tenCot, "late_fee_vnd_per_day")));
            dsPhiHuNhe.push_back(docSo(layGiaTri(cot, tenCot, "damage_fee_minor_vnd")));
            dsPhiHuNang.push_back(docSo(layGiaTri(cot, tenCot, "damage_fee_major_vnd")));
            dsChoThue.push_back(docCoKhong(layGiaTri(cot, tenCot, "rental_available"), true));
            dsChoBan.push_back(docCoKhong(layGiaTri(cot, tenCot, "sale_available"), f == 0));
        }
        file.close();
    }
    cout << "Đã đọc " << dsSanPham.size() << " sản phẩm từ file CSV." << endl;
    // Khách hàng mẫu để chạy thử ngay (có thể xóa 3 dòng này)
    

    // ===================== 3. MENU CHÍNH =====================
    int chon = -1;
    do {
        cout << "\n============ QUẢN LÝ & THU PHÍ TRUYỆN TRANH - BOARD GAME ============" << endl;
        cout << " --- SẢN PHẨM ---" << endl;
        cout << "  1. Danh sách sản phẩm" << endl;
        cout << "  2. Tìm sản phẩm (xem chi tiết)" << endl;
        cout << "  3. Tính giá mua lại truyện tranh" << endl;
        cout << " --- KHÁCH HÀNG ---" << endl;
        cout << "  4. Thêm khách hàng" << endl;
        cout << "  5. Sửa thông tin khách hàng" << endl;
        cout << "  6. Danh sách khách hàng" << endl;
        cout << " --- GIAO DỊCH ---" << endl;
        cout << "  7. Thuê sản phẩm" << endl;
        cout << "  8. Mua truyện tranh" << endl;
        cout << "  9. Trả sản phẩm" << endl;
        cout << " 10. Cập nhật phiếu quá hạn" << endl;
        cout << " 11. Danh sách phiếu giao dịch" << endl;
        cout << " --- THANH TOÁN ---" << endl;
        cout << " 12. Thanh toán" << endl;
        cout << " 13. Hoàn tiền" << endl;
        cout << " 14. Thống kê thanh toán" << endl;
        cout << " --- CẤU TRÚC DỮ LIỆU ---" << endl;
        cout << " 15. Demo Linked List" << endl;
        cout << " 16. Demo Stack" << endl;
        cout << " 17. Demo Queue" << endl;
        cout << " 18. Demo Map" << endl;
        cout << " 19. Demo BST" << endl;
        cout << " 20. Tìm kiếm sản phẩm" << endl;
        cout << " 21. Sắp xếp sản phẩm" << endl;
        cout << "  0. Thoát" << endl;
        cout << "Chọn chức năng: ";
        if (!getline(cin, s)) // hết dữ liệu nhập thì thoát
            break;
        if (!docSoNguyen(s, chon)) chon = -1;

        switch (chon) {

            // ---------- 1. Danh sách sản phẩm ----------
            case 1: {
                if (dsSanPham.empty())
                    cout << "Chưa có sản phẩm nào." << endl;
                cout << "(Giá thuê: truyện tranh tính theo tháng, board game tính theo giờ)" << endl;
                for (size_t i = 0; i < dsSanPham.size(); i++) {
                    Product* p = dsSanPham[i];
                    cout << p->getProductID() << " | " << p->getName() << " | Thuê: " << p->calculateRentalFee(1);
                    if (p->calculatePurchaseFee(1) > 0)
                        cout << " | Mua: " << p->calculatePurchaseFee(1);
                    else
                        cout << " | Không bán";
                    cout << " | SL: " << p->getQuantity() << endl;
                    if ((i + 1) % 20 == 0 && i + 1 < dsSanPham.size()) { // hết một trang 20 sản phẩm
                        cout << "-- Nhấn Enter để xem tiếp, gõ q để dừng -- ";
                        if (!getline(cin, s) || s == "q" || s == "Q") // hết dữ liệu nhập hoặc gõ q thì dừng
                            break;
                    }
                }
                break;
            }

            // ---------- 2. Tìm sản phẩm theo mã hoặc tên ----------
            case 2: {
                cout << "Nhập mã hoặc một phần tên cần tìm (phân biệt chữ hoa / thường): ";
                getline(cin, s);
                if (s.empty()) {
                    cout << "Chưa nhập từ khóa!" << endl;
                    break;
                }
                int dem = 0; // số sản phẩm tìm thấy
                for (size_t i = 0; i < dsSanPham.size(); i++) {
                    if (dsSanPham[i]->getProductID().find(s) != string::npos || dsSanPham[i]->getName().find(s) != string::npos) {
                        dem++;
                        if (dem <= 5) { // chỉ in chi tiết 5 sản phẩm đầu cho gọn
                            cout << "----------" << endl;
                            dsSanPham[i]->display(); // hàm ảo: tự in thông tin của ComicBook hoặc BoardGame
                        }
                    }
                }
                cout << "Tìm thấy " << dem << " sản phẩm." << endl;
                if (dem > 5)
                    cout << "(Chỉ hiển thị 5 sản phẩm đầu, hãy nhập từ khóa cụ thể hơn)" << endl;
                break;
            }

            // ---------- 3. Tính giá mua lại truyện tranh ----------
            case 3: {
                cout << "Mã truyện tranh (VD: CB00101): ";
                getline(cin, s);
                int sp = timViTriSanPham(dsSanPham, s);
                if (sp == -1 || dynamic_cast<ComicBook*>(dsSanPham[sp]) == nullptr) {
                    cout << "Không tìm thấy truyện tranh có mã này!" << endl;
                    break;
                }
                double giaGoc = dsSanPham[sp]->getPrice(); // giá bán gốc của truyện
                cout << "Giá bán gốc : " << giaGoc << " VND" << endl;
                cout << "Giá mua lại (" << tinhPhi.getBuybackRate() * 100 << "%): "
                     << tinhPhi.calculateBuybackPrice(giaGoc) << " VND" << endl; // FeeCalculator tính giá mua lại
                break;
            }

            // ---------- 4. Thêm khách hàng ----------
            case 4: {
                string ten, sdt, mail;
                cout << "Họ tên: ";
                getline(cin, ten);
                cout << "Số điện thoại: ";
                getline(cin, sdt);
                cout << "Email: ";
                getline(cin, mail);
                cout << "Loại khách (1: Thường, 2: VIP): ";
                getline(cin, s);
                if (ten.empty()) {
                    cout << "Họ tên không được để trống!" << endl;
                    break;
                }
                sprintf(ma, "KH%03d", (int)dsKhachHang.size() + 1); // tự tạo mã: KH004, KH005...
                if (s == "2")
                    dsKhachHang.push_back(new VIPCustomer(ma, ten, sdt, mail, 0));
                else
                    dsKhachHang.push_back(new Customer(ma, ten, sdt, mail));
                cout << "Đã thêm khách hàng:" << endl;
                dsKhachHang.back()->display(); // hàm ảo: in khách thường hoặc khách VIP
                break;
            }

            // ---------- 5. Sửa thông tin khách hàng ----------
            case 5: {
                cout << "Mã khách hàng cần sửa (VD: KH001): ";
                getline(cin, s);
                int kh = timViTriKhachHang(dsKhachHang, s); // vị trí khách hàng
                if (kh == -1) {
                    cout << "Không tìm thấy khách hàng!" << endl;
                    break;
                }
                string ten, sdt, mail;
                cout << "(Để trống = giữ nguyên)" << endl;
                cout << "Họ tên mới [" << dsKhachHang[kh]->getFullName() << "]: ";
                getline(cin, ten);
                cout << "SĐT mới [" << dsKhachHang[kh]->getPhoneNumber() << "]: ";
                getline(cin, sdt);
                cout << "Email mới [" << dsKhachHang[kh]->getEmail() << "]: ";
                getline(cin, mail);
                if (ten.empty())
                    ten = dsKhachHang[kh]->getFullName();
                if (sdt.empty())
                    sdt = dsKhachHang[kh]->getPhoneNumber();
                if (mail.empty())
                    mail = dsKhachHang[kh]->getEmail();
                dsKhachHang[kh]->setCustomerInfo(ten, sdt, mail); // class Customer: cập nhật thông tin
                cout << "Đã cập nhật:" << endl;
                dsKhachHang[kh]->display();
                break;
            }

            // ---------- 6. Danh sách khách hàng ----------
            case 6: {
                for (size_t i = 0; i < dsKhachHang.size(); i++) {
                    cout << "----------" << endl;
                    dsKhachHang[i]->display(); // hàm ảo: khách thường và khách VIP tự in đúng thông tin
                }
                break;
            }

            // ---------- 7. Thuê sản phẩm ----------
            case 7: {
                // Chọn khách hàng và sản phẩm
                cout << "Mã khách hàng (VD: KH001): ";
                getline(cin, s);
                int kh = timViTriKhachHang(dsKhachHang, s);
                cout << "Mã sản phẩm (VD: CB00101, BG0001): ";
                getline(cin, s);
                int sp = -1; // vị trí sản phẩm trong dsSanPham
                for (size_t i = 0; i < dsSanPham.size(); i++)
                    if (dsSanPham[i]->getProductID() == s)
                        sp = (int)i;
                if (kh == -1 || sp == -1) {
                    cout << "Không tìm thấy khách hàng hoặc sản phẩm!" << endl;
                    break;
                }
                if (static_cast<size_t>(sp) >= dsChoThue.size() || !dsChoThue[sp]) {
                    cout << "San pham nay duoc danh dau khong cho thue trong file CSV!" << endl;
                    break;
                }
                if (dsSanPham[sp]->getStatus() != AVAILABLE || dsSanPham[sp]->getQuantity() <= 0) {
                    cout << "Sản phẩm này hiện không còn để cho thuê!" << endl;
                    break;
                }

                // Nhập ngày thuê và thời gian thuê
                bool laTruyen = (dynamic_cast<ComicBook*>(dsSanPham[sp]) != nullptr);
                int d = 0, m = 0, y = 0;
                cout << "Ngày thuê (dd/mm/yyyy): ";
                getline(cin, s);
                if (!docNgay(s, d, m, y)) {
                    cout << "Ngay khong hop le. Hay nhap dd/mm/yyyy." << endl;
                    break;
                }
                cout << (laTruyen ? "Số tháng thuê (1-12): " : "Số giờ thuê (1-12): ");
                getline(cin, s);
                int thoiGian = 0;
                if (!docSoNguyen(s, thoiGian) || thoiGian < 1 || thoiGian > 12) {
                    cout << "Thời gian thuê không hợp lệ!" << endl;
                    break;
                }

                // Truyện tranh: hạn trả sau N tháng (nếu ngày không tồn tại ở tháng đích,
                // dùng ngày cuối tháng). Board game: thời hạn thuê theo giờ trong cùng ngày.
                Date ngayThue(d, m, y);
                Date hanTra = laTruyen ? congThang(ngayThue, thoiGian) : ngayThue;

                // Tính phí thuê và tạo phiếu
                double tienThue = tinhPhi.calculateRentalFee(dsSanPham[sp], thoiGian); // FeeCalculator tính phí thuê
                sprintf(ma, "GD%03d", (int)dsPhieu.size() + 1);
                Rental* phieu = new Rental(ma, dsKhachHang[kh], dsSanPham[sp], ngayThue, hanTra, RENTING);
                phieu->rentProduct(ngayThue, hanTra); // class Rental: ghi nhận ngày thuê, hạn trả, trạng thái RENTING
                dsPhieu.push_back(phieu);
                dsSoTien.push_back(tienThue);
                cout << "Mã phiếu: " << ma << " | Hạn trả: ";
                hanTra.display();
                cout << endl;
                cout << "Phí thuê: " << tienThue << " VND (phí trễ hạn / hư hỏng, nếu có, sẽ tính khi trả)" << endl;
                break;
            }

            // ---------- 8. Mua truyện tranh ----------
            case 8: {
                cout << "Mã khách hàng (VD: KH001): ";
                getline(cin, s);
                int kh = timViTriKhachHang(dsKhachHang, s);
                cout << "Mã truyện tranh (VD: CB00101): ";
                getline(cin, s);
                int sp = timViTriSanPham(dsSanPham, s);
                if (kh == -1 || sp == -1) {
                    cout << "Không tìm thấy khách hàng hoặc sản phẩm!" << endl;
                    break;
                }
                if (static_cast<size_t>(sp) >= dsChoBan.size() || !dsChoBan[sp] ||
                    tinhPhi.calculatePurchaseFee(dsSanPham[sp], 1) <= 0) {
                    cout << "Sản phẩm này không được phép bán!" << endl;
                    break;
                }
                if (dsSanPham[sp]->getStatus() != AVAILABLE || dsSanPham[sp]->getQuantity() <= 0) {
                    cout << "Sản phẩm hiện không ở trạng thái sẵn sàng để bán!" << endl;
                    break;
                }
                cout << "Số lượng mua (tối đa " << dsSanPham[sp]->getQuantity() << "): ";
                getline(cin, s);
                int soLuong = 0;
                if (!docSoNguyen(s, soLuong) || soLuong < 1 || soLuong > dsSanPham[sp]->getQuantity()) {
                    cout << "Số lượng không hợp lệ!" << endl;
                    break;
                }
                int d = 0, m = 0, y = 0;
                cout << "Ngày mua (dd/mm/yyyy): ";
                getline(cin, s);
                if (!docNgay(s, d, m, y)) {
                    cout << "Ngay khong hop le. Hay nhap dd/mm/yyyy." << endl;
                    break;
                }
                Date ngayMua(d, m, y);
                double tien = tinhPhi.calculatePurchaseFee(dsSanPham[sp], soLuong); // FeeCalculator tính tiền mua
                sprintf(ma, "GD%03d", (int)dsPhieu.size() + 1);
                Rental* phieu = new Rental(ma, dsKhachHang[kh], dsSanPham[sp], ngayMua, ngayMua, PURCHASED);
                phieu->buyProduct(ngayMua); // class Rental: ghi nhận ngày mua, trạng thái PURCHASED
                dsPhieu.push_back(phieu);
                dsSoTien.push_back(tien);
                cout << "Mã phiếu: " << ma << " | Thành tiền: " << tien << " VND (vào mục Thanh toán để thu tiền)" << endl;
                break;
            }

            // ---------- 9. Trả sản phẩm ----------
            case 9: {
                cout << "Mã phiếu cần trả (VD: GD001): ";
                getline(cin, s);
                int pi = timViTriPhieu(dsPhieu, s); // vị trí phiếu trong dsPhieu
                if (pi == -1) {
                    cout << "Không tìm thấy phiếu!" << endl;
                    break;
                }
                Rental* phieu = dsPhieu[pi];
                if (phieu->getStatus() != RENTING && phieu->getStatus() != OVERDUE) {
                    cout << "Phiếu này không ở trạng thái đang thuê!" << endl;
                    break;
                }
                int d = 0, m = 0, y = 0;
                cout << "Ngày trả thực tế (dd/mm/yyyy): ";
                getline(cin, s);
                if (!docNgay(s, d, m, y)) {
                    cout << "Ngay khong hop le. Hay nhap dd/mm/yyyy." << endl;
                    break;
                }
                // Ngày trả không được trước ngày thuê.
                Date ngayThue = phieu->getRentalDate();
                if (ngayThanhSo(y, m, d) < ngayThanhSo(ngayThue.getYear(), ngayThue.getMonth(), ngayThue.getDay())) {
                    cout << "Ngày trả không được trước ngày thuê!" << endl;
                    break;
                }

                // Số ngày trễ = số ngày lịch giữa ngày trả và hạn trả.
                Date hanTra = phieu->getDueDate();
                long long chenhlechNgay = ngayThanhSo(y, m, d) -
                    ngayThanhSo(hanTra.getYear(), hanTra.getMonth(), hanTra.getDay());
                int soNgayTre = chenhlechNgay > 0 ? static_cast<int>(chenhlechNgay) : 0;

                // Tìm mức phí riêng của sản phẩm trên các mảng song song.
                int sp = -1;
                for (size_t i = 0; i < dsSanPham.size(); ++i) {
                    if (dsSanPham[i] == phieu->getProduct()) {
                        sp = static_cast<int>(i);
                        break;
                    }
                }
                if (sp < 0 || static_cast<size_t>(sp) >= dsPhiTre.size() ||
                    static_cast<size_t>(sp) >= dsPhiHuNhe.size() ||
                    static_cast<size_t>(sp) >= dsPhiHuNang.size()) {
                    cout << "Khong tim duoc bang phi cua san pham; chua cap nhat phieu tra." << endl;
                    break;
                }
                double phiTre = tinhPhi.calculateLateFee(soNgayTre, dsPhiTre[sp]);

                cout << "Tình trạng sản phẩm khi trả (1: Bình thường, 2: Hư hỏng nhẹ, 3: Hư hỏng nặng): ";
                getline(cin, s);
                if (s != "1" && s != "2" && s != "3") {
                    cout << "Lua chon tinh trang khong hop le; chua cap nhat phieu tra." << endl;
                    break;
                }
                double phiHuHong = 0;
                if (s == "2")
                    phiHuHong = tinhPhi.calculateDamageFee(dsPhiHuNhe[sp]);
                else if (s == "3")
                    phiHuHong = tinhPhi.calculateDamageFee(dsPhiHuNang[sp]);

                // Tổng = phí thuê (đã lưu lúc thuê) + phí trễ + phí hư hỏng
                double tong = tinhPhi.calculateTotalFee(dsSoTien[pi], phiTre, phiHuHong);
                phieu->returnProduct(Date(d, m, y)); // class Rental: ghi nhận ngày trả, trạng thái RETURNED
                cout << "Phí thuê: " << dsSoTien[pi] << " | Phí trễ (" << soNgayTre << " ngày): " << phiTre
                     << " | Phí hư hỏng: " << phiHuHong << endl;
                cout << "TỔNG CẦN THANH TOÁN: " << tong << " VND (chưa VAT) - vào mục Thanh toán để thu tiền" << endl;
                dsSoTien[pi] = tong; // lưu lại tổng tiền để lát nữa thanh toán
                break;
            }

            // ---------- 10. Cập nhật phiếu quá hạn ----------
            case 10: {
                int d = 0, m = 0, y = 0;
                cout << "Ngày kiểm tra (dd/mm/yyyy): ";
                getline(cin, s);
                if (!docNgay(s, d, m, y)) {
                    cout << "Ngay khong hop le. Hay nhap dd/mm/yyyy." << endl;
                    break;
                }
                long long homNay = ngayThanhSo(y, m, d);
                int dem = 0; // số phiếu bị chuyển sang quá hạn
                for (size_t i = 0; i < dsPhieu.size(); i++) {
                    Date han = dsPhieu[i]->getDueDate();
                    if (dsPhieu[i]->getStatus() == RENTING &&
                        homNay > ngayThanhSo(han.getYear(), han.getMonth(), han.getDay())) {
                        dsPhieu[i]->setOverdue(); // class Rental: đổi trạng thái từ RENTING sang OVERDUE
                        dem++;
                    }
                }
                cout << "Có " << dem << " phiếu bị chuyển sang quá hạn." << endl;
                break;
            }

            // ---------- 11. Danh sách phiếu giao dịch ----------
            case 11: {
                if (dsPhieu.empty()) {
                    cout << "Chưa có phiếu giao dịch nào." << endl;
                    break;
                }
                const char* tenTrangThai[] = {"Đang thuê", "Đã trả", "Quá hạn", "Đã mua"}; // đúng thứ tự enum RentalStatus
                for (size_t i = 0; i < dsPhieu.size(); i++) {
                    Rental* r = dsPhieu[i];
                    cout << r->getRentalId() << " | " << r->getCustomer()->getFullName() << " | "
                         << r->getProduct()->getName() << " | " << tenTrangThai[r->getStatus()] << " | Hạn trả: ";
                    if (r->getStatus() == PURCHASED)
                        cout << "-";
                    else
                        r->getDueDate().display();
                    cout << " | Tiền: " << dsSoTien[i] << endl;
                }
                cout << "Nhập mã phiếu để xem chi tiết (Enter = bỏ qua): ";
                getline(cin, s);
                for (size_t i = 0; i < dsPhieu.size(); i++)
                    if (dsPhieu[i]->getRentalId() == s) {
                        cout << "----------" << endl;
                        dsPhieu[i]->display(); // class Rental: in phiếu kèm thông tin khách hàng và sản phẩm
                    }
                break;
            }

            // ---------- 12. Thanh toán ----------
            case 12: {
                cout << "Mã phiếu cần thanh toán (VD: GD001): ";
                getline(cin, s);
                int pi = timViTriPhieu(dsPhieu, s);
                if (pi == -1) {
                    cout << "Không tìm thấy phiếu!" << endl;
                    break;
                }
                Rental* phieu = dsPhieu[pi];
                if (phieu->getStatus() != RETURNED && phieu->getStatus() != PURCHASED) {
                    cout << "Phiếu chưa thể thanh toán (khách chưa trả sản phẩm)!" << endl;
                    break;
                }
                bool daThanhToan = false; // kiểm tra phiếu này đã có thanh toán chưa
                for (size_t i = 0; i < dsThanhToan.size(); i++)
                    if (dsThanhToan[i]->getRentalId() == phieu->getRentalId())
                        daThanhToan = true;
                if (daThanhToan) {
                    cout << "Phiếu này đã được thanh toán trước đó!" << endl;
                    break;
                }

                cout << "Số tiền cần thanh toán: " << dsSoTien[pi] << " VND" << endl;
                cout << "Phương thức (1: Tiền mặt, 2: Chuyển khoản - VAT 3%, 3: Khác): ";
                getline(cin, s);
                PaymentMethod pt;
                if (s == "1") pt = CASH;
                else if (s == "2") pt = BANK_TRANSFER;
                else if (s == "3") pt = OTHER_METHOD;
                else {
                    cout << "Phuong thuc thanh toan khong hop le; giao dich chua duoc tao." << endl;
                    break;
                }

                // Ngày thanh toán = ngày trả (phiếu thuê) hoặc ngày mua (phiếu mua)
                Date ngayTT = (phieu->getStatus() == RETURNED) ? phieu->getReturnDate() : phieu->getRentalDate();
                sprintf(ma, "TT%03d", (int)dsThanhToan.size() + 1);
                Payment* tt = new Payment(ma, phieu->getRentalId(), dsSoTien[pi], pt, ngayTT);
                tt->makePayment(tinhPhi); // class Payment: FeeCalculator tính VAT, trạng thái thành PAID
                dsThanhToan.push_back(tt);
                tt->display();

                // Khách VIP được cộng điểm tích lũy: mỗi 10.000 VND = 1 điểm
                VIPCustomer* vip = dynamic_cast<VIPCustomer*>(phieu->getCustomer());
                if (vip != nullptr) {
                    vip->addPoints((int)(tt->getTotalAmount() / 10000));
                    cout << "Khách VIP, điểm tích lũy hiện có: " << vip->getMembershipPoints() << " điểm" << endl;
                }
                break;
            }

            // ---------- 13. Hoàn tiền ----------
            case 13: {
                cout << "Mã thanh toán cần hoàn (VD: TT001): ";
                getline(cin, s);
                int ti = timViTriThanhToan(dsThanhToan, s); // vị trí thanh toán
                if (ti == -1) {
                    cout << "Không tìm thấy thanh toán!" << endl;
                    break;
                }
                if (dsThanhToan[ti]->getStatus() != PAID) {
                    cout << "Chỉ hoàn tiền được cho thanh toán đang ở trạng thái \"Đã thanh toán\"!" << endl;
                    break;
                }
                dsThanhToan[ti]->refundPayment(); // class Payment: đổi trạng thái thành REFUNDED
                cout << "Đã hoàn " << dsThanhToan[ti]->getTotalAmount() << " VND cho khách." << endl;
                break;
            }

            // ---------- 14. Thống kê thanh toán ----------
            case 14: {
                PaymentStatistics thongKe;
                for (size_t i = 0; i < dsThanhToan.size(); i++)
                    thongKe.recordPayment(*dsThanhToan[i]); // chỉ ghi nhận các thanh toán đã PAID
                thongKe.displayStatistics();
                break;
            }

            // ---------- 15. Demo Linked List ----------
            case 15: {
                LinkedListString ll;
                cout << "=== Demo Linked List ===" << endl;
                ll.insertFront("Doraemon");
                ll.insertFront("One Piece");
                ll.insertBack("Naruto");
                ll.insertBack("Dragon Ball");
                ll.display();
                cout << "Kích thước: " << ll.getSize() << endl;
                cout << "Chứa 'Naruto': " << (ll.contains("Naruto") ? "Có" : "Không") << endl;
                ll.remove("One Piece");
                cout << "Sau khi xóa 'One Piece':" << endl;
                ll.display();
                break;
            }

            // ---------- 16. Demo Stack ----------
            case 16: {
                StackInt st;
                cout << "=== Demo Stack ===" << endl;
                st.push(10);
                st.push(20);
                st.push(30);
                st.display();
                int val;
                if (st.topValue(val)) {
                    cout << "Top: " << val << endl;
                }
                if (st.pop(val)) {
                    cout << "Pop: " << val << endl;
                }
                st.display();
                break;
            }

            // ---------- 17. Demo Queue ----------
            case 17: {
                QueueString q;
                cout << "=== Demo Queue ===" << endl;
                q.enqueue("Khách 1");
                q.enqueue("Khách 2");
                q.enqueue("Khách 3");
                q.display();
                string val;
                if (q.dequeue(val)) {
                    cout << "Dequeue: " << val << endl;
                }
                q.display();
                break;
            }

            // ---------- 18. Demo Map ----------
            case 18: {
                SimpleMapStringInt mp;
                cout << "=== Demo Map ===" << endl;
                mp.insert("CB00101", 48000);
                mp.insert("CB00102", 55000);
                mp.insert("BG0001", 950000);
                mp.display();
                int gia;
                if (mp.find("CB00101", gia)) {
                    cout << "Giá CB00101: " << gia << " VND" << endl;
                }
                cout << "Kích thước: " << mp.getSize() << endl;
                break;
            }

            // ---------- 19. Demo BST ----------
            case 19: {
                BinarySearchTreeInt bst;
                cout << "=== Demo Binary Search Tree ===" << endl;
                bst.insert(50);
                bst.insert(30);
                bst.insert(70);
                bst.insert(20);
                bst.insert(40);
                bst.insert(60);
                bst.insert(80);
                bst.display();
                cout << "Chứa 40: " << (bst.contains(40) ? "Có" : "Không") << endl;
                cout << "Kích thước: " << bst.getSize() << endl;
                break;
            }

            // ---------- 20. Tìm kiếm sản phẩm ----------
            case 20: {
                cout << "=== Tìm kiếm sản phẩm ===" << endl;
                if (dsSanPham.empty()) {
                    cout << "Chua co san pham de tim kiem." << endl;
                    break;
                }
                // Tạo mảng mã tạm; không làm thay đổi thứ tự danh sách sản phẩm gốc.
                vector<string> dsID;
                dsID.reserve(dsSanPham.size());
                for (size_t i = 0; i < dsSanPham.size(); ++i) {
                    if (dsSanPham[i] != nullptr) dsID.push_back(dsSanPham[i]->getProductID());
                }
                int n = static_cast<int>(dsID.size());
                
                cout << "Chọn phương pháp tìm kiếm:" << endl;
                cout << " 1. Tìm kiếm tuyến tính" << endl;
                cout << " 2. Tìm kiếm nhị phân (cần sắp xếp trước)" << endl;
                cout << "Lựa chọn: ";
                getline(cin, s);
                
                cout << "Nhập mã sản phẩm cần tìm: ";
                string maTim;
                getline(cin, maTim);
                
                int viTri = -1;
                if (s == "1") {
                    viTri = linearSearchString(dsID.empty() ? nullptr : &dsID[0], n, maTim);
                    cout << "Tìm kiếm tuyến tính: ";
                } else if (s == "2") {
                    // Sắp xếp trước khi tìm nhị phân
                    bubbleSortString(dsID.empty() ? nullptr : &dsID[0], n);
                    viTri = binarySearchString(dsID.empty() ? nullptr : &dsID[0], n, maTim);
                    cout << "Tìm kiếm nhị phân: ";
                } else {
                    cout << "Phương pháp không hợp lệ!" << endl;
                    break;
                }
                
                if (viTri != -1) {
                    cout << "Tìm thấy trong mảng mã tạm tại vị trí " << viTri << endl;
                    // Tìm sản phẩm tương ứng
                    for (size_t i = 0; i < dsSanPham.size(); i++) {
                        if (dsSanPham[i]->getProductID() == maTim) {
                            dsSanPham[i]->display();
                            break;
                        }
                    }
                } else {
                    cout << "Không tìm thấy sản phẩm!" << endl;
                }
                break;
            }

            // ---------- 21. Sắp xếp sản phẩm ----------
            case 21: {
                cout << "=== SAP XEP SAN PHAM ===" << endl;
                if (dsSanPham.empty()) {
                    cout << "Chua co san pham de sap xep." << endl;
                    break;
                }

                cout << "Chon tieu chi sap xep:" << endl;
                cout << " 1. Theo ten (A-Z)" << endl;
                cout << " 2. Theo ma san pham (A-Z)" << endl;
                cout << " 3. Theo so luong (it den nhieu)" << endl;
                cout << " 4. Theo so luong (nhieu den it)" << endl;
                cout << " 5. Theo trang thai (co san -> dang thue -> hu hong)" << endl;
                cout << " 6. Theo gia thue truyen tranh (VND/thang)" << endl;
                cout << " 7. Theo gia thue board game (VND/gio)" << endl;
                cout << "Lua chon: ";
                getline(cin, s);

                ProductSortCriteria tieuChi;
                vector<Product*> dsTam;
                bool sapXepTheoGiaThue = false;
                bool laTruyenTranh = false;

                if (s == "1") {
                    tieuChi = PRODUCT_SORT_NAME_ASC;
                    dsTam = dsSanPham;
                } else if (s == "2") {
                    tieuChi = PRODUCT_SORT_ID_ASC;
                    dsTam = dsSanPham;
                } else if (s == "3") {
                    tieuChi = PRODUCT_SORT_QUANTITY_ASC;
                    dsTam = dsSanPham;
                } else if (s == "4") {
                    tieuChi = PRODUCT_SORT_QUANTITY_DESC;
                    dsTam = dsSanPham;
                } else if (s == "5") {
                    tieuChi = PRODUCT_SORT_STATUS_ASC;
                    dsTam = dsSanPham;
                } else if (s == "6") {
                    // Chỉ sắp xếp truyện tranh: giá thuê tính theo tháng.
                    tieuChi = PRODUCT_SORT_RENTAL_PRICE_ASC;
                    dsTam = getComicBooks(dsSanPham);
                    sapXepTheoGiaThue = true;
                    laTruyenTranh = true;
                } else if (s == "7") {
                    // Chỉ sắp xếp board game: giá thuê tính theo giờ.
                    tieuChi = PRODUCT_SORT_RENTAL_PRICE_ASC;
                    dsTam = getBoardGames(dsSanPham);
                    sapXepTheoGiaThue = true;
                } else {
                    cout << "Tieu chi khong hop le!" << endl;
                    break;
                }

                if (dsTam.empty()) {
                    cout << "Khong co san pham thuoc nhom nay." << endl;
                    break;
                }

                cout << "Chon thuat toan:" << endl;
                cout << " 1. Bubble Sort" << endl;
                cout << " 2. Quick Sort" << endl;
                cout << "Lua chon: ";
                getline(cin, s);

                if (s == "1") {
                    // Phần cài đặt Bubble Sort nằm trong data_structures.cpp.
                    bubbleSortProducts(dsTam, tieuChi);
                    cout << "Da sap xep bang Bubble Sort." << endl;
                } else if (s == "2") {
                    // Phần cài đặt Quick Sort nằm trong data_structures.cpp.
                    quickSortProducts(dsTam, tieuChi);
                    cout << "Da sap xep bang Quick Sort." << endl;
                } else {
                    cout << "Thuat toan khong hop le!" << endl;
                    break;
                }

                cout << "\nDanh sach sau khi sap xep (toi da 20 san pham):" << endl;
                int soHien = (dsTam.size() < 20)
                    ? static_cast<int>(dsTam.size()) : 20;

                for (int i = 0; i < soHien; ++i) {
                    Product* p = dsTam[i];
                    if (p == nullptr) continue;

                    cout << p->getProductID() << " | " << p->getName()
                         << " | SL: " << p->getQuantity();

                    if (sapXepTheoGiaThue) {
                        cout << " | Gia thue: " << p->calculateRentalFee(1)
                             << (laTruyenTranh ? " VND/thang" : " VND/gio");
                    }
                    cout << endl;
                }

                if (static_cast<int>(dsTam.size()) > soHien) {
                    cout << "... va " << (dsTam.size() - soHien)
                         << " san pham khac." << endl;
                }

                cout << "\nLuu y: danh sach goc khong bi thay doi; day la ket qua sap xep tren danh sach tam." << endl;
                break;
            }

            case 0:
                cout << "Tạm biệt!" << endl;
                break;

            default:
                cout << "Lựa chọn không hợp lệ, hãy nhập số từ 0 đến 21!" << endl;
        }
    } while (chon != 0);

    // ===================== 4. GIẢI PHÓNG BỘ NHỚ =====================
    for (size_t i = 0; i < dsThanhToan.size(); i++)
        delete dsThanhToan[i];
    for (size_t i = 0; i < dsPhieu.size(); i++)
        delete dsPhieu[i];
    for (size_t i = 0; i < dsKhachHang.size(); i++)
        delete dsKhachHang[i];
    for (size_t i = 0; i < dsSanPham.size(); i++)
        delete dsSanPham[i];
    return 0;
}

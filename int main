
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include "class_payment_FeeCalculator.cpp" // đã gồm sẵn 2 file class còn lại
using namespace std;

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
            int soLuong = docSo(layGiaTri(cot, tenCot, "quantity"));
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
        cout << "  0. Thoát" << endl;
        cout << "Chọn chức năng: ";
        if (!getline(cin, s)) // hết dữ liệu nhập thì thoát
            break;
        chon = (s.empty() || !isdigit((unsigned char)s[0])) ? -1 : atoi(s.c_str());

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
                int sp = -1; // vị trí sản phẩm trong dsSanPham
                for (size_t i = 0; i < dsSanPham.size(); i++)
                    if (dsSanPham[i]->getProductID() == s)
                        sp = (int)i;
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
                int kh = -1; // vị trí khách hàng trong dsKhachHang
                for (size_t i = 0; i < dsKhachHang.size(); i++)
                    if (dsKhachHang[i]->getCustomerId() == s)
                        kh = (int)i;
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
                int kh = -1; // vị trí khách hàng trong dsKhachHang
                for (size_t i = 0; i < dsKhachHang.size(); i++)
                    if (dsKhachHang[i]->getCustomerId() == s)
                        kh = (int)i;
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
                if (dsSanPham[sp]->getStatus() != AVAILABLE || dsSanPham[sp]->getQuantity() <= 0) {
                    cout << "Sản phẩm này hiện không còn để cho thuê!" << endl;
                    break;
                }

                // Nhập ngày thuê và thời gian thuê
                bool laTruyen = (dynamic_cast<ComicBook*>(dsSanPham[sp]) != nullptr);
                int d = 0, m = 0, y = 0;
                cout << "Ngày thuê (dd/mm/yyyy): ";
                getline(cin, s);
                if (sscanf(s.c_str(), "%d%*[/ ]%d%*[/ ]%d", &d, &m, &y) != 3 || d < 1 || d > 31 || m < 1 || m > 12 || y < 1970) {
                    cout << "Ngày không hợp lệ!" << endl;
                    break;
                }
                cout << (laTruyen ? "Số tháng thuê (1-12): " : "Số giờ thuê (1-12): ");
                getline(cin, s);
                int thoiGian = atoi(s.c_str());
                if (thoiGian < 1 || thoiGian > 12) {
                    cout << "Thời gian thuê không hợp lệ!" << endl;
                    break;
                }

                // Tính hạn trả: truyện tranh = ngày thuê + số tháng,
                // board game thuê theo giờ nên phải trả trong ngày
                tm han = tm();
                han.tm_mday = d;
                han.tm_mon = m - 1;
                han.tm_year = y - 1900;
                han.tm_hour = 12;
                if (laTruyen) {
                    han.tm_mon += thoiGian;
                    mktime(&han); // tự chuẩn hóa tháng / năm (VD: tháng 14 thành tháng 2 năm sau)
                    if (han.tm_mday != d) { // ngày bị tràn sang tháng sau (VD: 31/01 + 1 tháng) thì lùi về ngày cuối tháng
                        han.tm_mday = 0;
                        mktime(&han);
                    }
                }
                Date ngayThue(d, m, y);
                Date hanTra(han.tm_mday, han.tm_mon + 1, han.tm_year + 1900);

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
                int kh = -1;
                for (size_t i = 0; i < dsKhachHang.size(); i++)
                    if (dsKhachHang[i]->getCustomerId() == s)
                        kh = (int)i;
                cout << "Mã truyện tranh (VD: CB00101): ";
                getline(cin, s);
                int sp = -1;
                for (size_t i = 0; i < dsSanPham.size(); i++)
                    if (dsSanPham[i]->getProductID() == s)
                        sp = (int)i;
                if (kh == -1 || sp == -1) {
                    cout << "Không tìm thấy khách hàng hoặc sản phẩm!" << endl;
                    break;
                }
                if (tinhPhi.calculatePurchaseFee(dsSanPham[sp], 1) <= 0) { // board game không bán nên phí mua = 0
                    cout << "Sản phẩm này không bán (board game chỉ cho thuê)!" << endl;
                    break;
                }
                cout << "Số lượng mua (tối đa " << dsSanPham[sp]->getQuantity() << "): ";
                getline(cin, s);
                int soLuong = atoi(s.c_str());
                if (soLuong < 1 || soLuong > dsSanPham[sp]->getQuantity()) {
                    cout << "Số lượng không hợp lệ!" << endl;
                    break;
                }
                int d = 0, m = 0, y = 0;
                cout << "Ngày mua (dd/mm/yyyy): ";
                getline(cin, s);
                if (sscanf(s.c_str(), "%d%*[/ ]%d%*[/ ]%d", &d, &m, &y) != 3 || d < 1 || d > 31 || m < 1 || m > 12 || y < 1970) {
                    cout << "Ngày không hợp lệ!" << endl;
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
                int pi = -1; // vị trí phiếu trong dsPhieu
                for (size_t i = 0; i < dsPhieu.size(); i++)
                    if (dsPhieu[i]->getRentalId() == s)
                        pi = (int)i;
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
                if (sscanf(s.c_str(), "%d%*[/ ]%d%*[/ ]%d", &d, &m, &y) != 3 || d < 1 || d > 31 || m < 1 || m > 12 || y < 1970) {
                    cout << "Ngày không hợp lệ!" << endl;
                    break;
                }
                // Ngày trả không được trước ngày thuê (đổi ngày thành số dạng yyyymmdd để so sánh)
                Date ngayThue = phieu->getRentalDate();
                if (y * 10000 + m * 100 + d < ngayThue.getYear() * 10000 + ngayThue.getMonth() * 100 + ngayThue.getDay()) {
                    cout << "Ngày trả không được trước ngày thuê!" << endl;
                    break;
                }

                // Số ngày trễ = ngày trả thực tế - hạn trả.
                // Đổi hai ngày sang kiểu thời gian (mktime) rồi trừ nhau ra số giây, chia 86400 ra số ngày
                Date hanTra = phieu->getDueDate();
                tm han = tm();
                tm tra = tm();
                han.tm_mday = hanTra.getDay();
                han.tm_mon = hanTra.getMonth() - 1;
                han.tm_year = hanTra.getYear() - 1900;
                han.tm_hour = 12;
                tra.tm_mday = d;
                tra.tm_mon = m - 1;
                tra.tm_year = y - 1900;
                tra.tm_hour = 12;
                int soNgayTre = (int)(difftime(mktime(&tra), mktime(&han)) / 86400 + 0.5);
                if (soNgayTre < 0)
                    soNgayTre = 0; // trả đúng hạn hoặc sớm hơn thì không bị phạt

                // Tìm vị trí sản phẩm trong dsSanPham để lấy mức phí riêng của sản phẩm đó
                int sp = 0;
                for (size_t i = 0; i < dsSanPham.size(); i++)
                    if (dsSanPham[i] == phieu->getProduct())
                        sp = (int)i;
                double phiTre = tinhPhi.calculateLateFee(soNgayTre, dsPhiTre[sp]); // phí trễ = số ngày trễ x phí mỗi ngày

                cout << "Tình trạng sản phẩm khi trả (1: Bình thường, 2: Hư hỏng nhẹ, 3: Hư hỏng nặng): ";
                getline(cin, s);
                double phiHuHong = 0; // mặc định không hư hỏng
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
                if (sscanf(s.c_str(), "%d%*[/ ]%d%*[/ ]%d", &d, &m, &y) != 3 || d < 1 || d > 31 || m < 1 || m > 12 || y < 1970) {
                    cout << "Ngày không hợp lệ!" << endl;
                    break;
                }
                int homNay = y * 10000 + m * 100 + d; // đổi ngày thành số dạng yyyymmdd để so sánh
                int dem = 0; // số phiếu bị chuyển sang quá hạn
                for (size_t i = 0; i < dsPhieu.size(); i++) {
                    Date han = dsPhieu[i]->getDueDate();
                    if (dsPhieu[i]->getStatus() == RENTING && homNay > han.getYear() * 10000 + han.getMonth() * 100 + han.getDay()) {
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
                int pi = -1;
                for (size_t i = 0; i < dsPhieu.size(); i++)
                    if (dsPhieu[i]->getRentalId() == s)
                        pi = (int)i;
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
                PaymentMethod pt = CASH;
                if (s == "2")
                    pt = BANK_TRANSFER;
                else if (s == "3")
                    pt = OTHER_METHOD;

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
                int ti = -1; // vị trí thanh toán trong dsThanhToan
                for (size_t i = 0; i < dsThanhToan.size(); i++)
                    if (dsThanhToan[i]->getPaymentId() == s)
                        ti = (int)i;
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

            case 0:
                cout << "Tạm biệt!" << endl;
                break;

            default:
                cout << "Lựa chọn không hợp lệ, hãy nhập số từ 0 đến 14!" << endl;
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

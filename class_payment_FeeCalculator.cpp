
#ifndef CLASS_PAYMENT_FEECALCULATOR_CPP
#define CLASS_PAYMENT_FEECALCULATOR_CPP

#include <iostream>
#include <string>
#include <sstream>
using namespace std;

// PHẦN 1: CÁC KIỂU DỮ LIỆU DÙNG CHUNG

// Trạng thái của giao dịch thanh toán
enum class PaymentStatus {
    UNPAID,     // Chưa thanh toán
    PAID,       // Đã thanh toán
    REFUNDED    // Đã hoàn tiền
};
// Kết quả trả về sau khi thực hiện thanh toán
struct PaymentResult {
    bool success;       // Thanh toán thành công hay thất bại
    string message;     // Thông báo kết quả
    // Hàm khởi tạo kết quả thanh toán
    PaymentResult(bool s = false, string m = "")
        : success(s), message(m) {}
};

// PHẦN 2: PAYMENT - LỚP THANH TOÁN TRỪU TƯỢNG

class Payment {
private:
    string paymentID;           // Mã thanh toán
    string transactionID;       // Mã giao dịch liên quan
    double amount;              // Số tiền ban đầu
    double transactionFee;      // Phí giao dịch
    double totalAmount;         // Tổng tiền phải thanh toán
    string paymentDate;         // Ngày thanh toán
    PaymentStatus status;       // Trạng thái thanh toán

protected:
    // Cập nhật tổng tiền sau khi tính phí giao dịch
    void updateTotalAmount() {
        totalAmount = calculateTotalAmount();
    }

    // Cho phép lớp con cập nhật trạng thái thanh toán
    void updateStatus(PaymentStatus s) {
        status = s;
    }

public:
    // Constructor khởi tạo thông tin thanh toán
    Payment(string id, string tid, double a) : paymentID(id), transactionID(tid),
amount(a), transactionFee(0),  totalAmount(a),
paymentDate(""), status(PaymentStatus::UNPAID) {}
    // Destructor ảo của lớp trừu tượng
    virtual ~Payment() {}
    // Lấy mã thanh toán
    string getPaymentID() const {
        return paymentID;
    }
    // Lấy mã giao dịch
    string getTransactionID() const {
        return transactionID;
    }
    // Lấy số tiền ban đầu
    double getAmount() const {
        return amount;
    }
    // Lấy tổng số tiền cần thanh toán
    double getTotalAmount() const {
        return totalAmount;
    }
    // Lấy trạng thái thanh toán
    PaymentStatus getStatus() const {
        return status;
    }
    // Cập nhật trạng thái thanh toán
    void setStatus(PaymentStatus s) {
        // Không cho phép chuyển giao dịch đã hoàn tiền thành trạng thái khác
        if (status == PaymentStatus::REFUNDED) {
            return;
        }
        status = s;
    }
    // Cập nhật phí giao dịch
    void setTransactionFee(double fee) {
        if (fee >= 0) {
            transactionFee = fee;
            updateTotalAmount();
        }
    }
    // Tính tổng tiền = tiền ban đầu + phí giao dịch
    double calculateTotalAmount() {
        totalAmount = amount + transactionFee;
        return totalAmount;
    }

    // Phương thức thuần ảo: lớp con bắt buộc phải cài đặt
    virtual PaymentResult processPayment() = 0;

    // Hoàn tiền nếu giao dịch đã thanh toán
    virtual bool refundPayment() {
        if (status != PaymentStatus::PAID) {
            return false;
        }
        status = PaymentStatus::REFUNDED;
        return true;
    }

    // Hiển thị thông tin thanh toán
    virtual void displayPayment() {
        cout << "\nMa thanh toan: " << paymentID;
        cout << "\nMa giao dich: " << transactionID;
        cout << "\nSo tien ban dau: " << amount;
        cout << "\nPhi giao dich: " << transactionFee;
        cout << "\nTong tien: " << calculateTotalAmount();
        cout << "\nTrang thai: ";

        if (status == PaymentStatus::UNPAID)
            cout << "Chua thanh toan";
        else if (status == PaymentStatus::PAID)
            cout << "Da thanh toan";
        else
            cout << "Da hoan tien";
        cout << endl;
    }
};

// PHẦN 3: CASHPAYMENT - THANH TOÁN TIỀN MẶT

class CashPayment : public Payment {
private:
    double receivedAmount;  // Số tiền khách đưa
    double changeAmount;    // Số tiền thừa

public:
    // Constructor khởi tạo thanh toán tiền mặt
    CashPayment(string id, string tid, double a, double received) : Payment(id, tid, a), 
receivedAmount(received), changeAmount(0) {}
    // Lấy số tiền khách đã đưa
    double getReceivedAmount() const {
        return receivedAmount;
    }
    // Cập nhật số tiền khách đưa
    void setReceivedAmount(double received) {
        if (received >= 0) {
            receivedAmount = received;
        }
    }
    // Kiểm tra số tiền khách đưa có đủ hay không
    virtual bool verifyCashAmount() {
        return receivedAmount >= calculateTotalAmount();
    }
    // Tính tiền thừa
    virtual double calculateChange() {
        if (!verifyCashAmount()) {
            changeAmount = 0;
            return changeAmount;
        }
        changeAmount = receivedAmount - calculateTotalAmount();
        return changeAmount;
    }
    // Xử lý thanh toán tiền mặt
    virtual PaymentResult processPayment() override {
        // Kiểm tra số tiền khách đưa
        if (!verifyCashAmount()) {
            return PaymentResult(false, "So tien khach dua khong du");
        }
        // Tính tiền thừa
        calculateChange();
        // Cập nhật trạng thái đã thanh toán
        setStatus(PaymentStatus::PAID);
        return PaymentResult(true, "Thanh toan tien mat thanh cong");
    }
    // Hoàn tiền mặt
    virtual bool refundPayment() override {
        return Payment::refundPayment();
    }
    // Hiển thị thông tin thanh toán tiền mặt
    virtual void displayPayment() override {
        Payment::displayPayment();
        cout << "Tien khach dua: " << receivedAmount << endl;
        cout << "Tien thua: " << changeAmount << endl;
    }
};

// PHẦN 4: CASHATCOUNTER - THANH TOÁN TẠI QUẦY

class CashAtCounter : public CashPayment {
private:
    string counterID;       // Mã quầy
    string staffID;         // Mã nhân viên
    string cashierName;     // Tên nhân viên thu ngân

public:
    // Constructor thanh toán tại quầy
    CashAtCounter(string id, string tid, double a,
                  string cid, string sid, string name)
        : CashPayment(id, tid, a, a),
          counterID(cid), staffID(sid),
          cashierName(name) {}

    // Kiểm tra số tiền khách đưa
    virtual bool verifyCashAmount() override {
        return CashPayment::verifyCashAmount();
    }
    // Tính tiền thừa
    virtual double calculateChange() override {
        return CashPayment::calculateChange();
    }
    // In hóa đơn thanh toán tại quầy
    void printReceipt() {
        cout << "\n========== HOA DON THANH TOAN ==========";
        cout << "\nMa quay: " << counterID;
        cout << "\nMa nhan vien: " << staffID;
        cout << "\nThu ngan: " << cashierName;
        cout << "\nMa thanh toan: " << getPaymentID();
        cout << "\nTong tien: " << getTotalAmount();
        cout << "\nTien khach dua: " << getReceivedAmount();
        cout << "\nTien thua: " << calculateChange();
        cout << "\n========================================\n";
    }

    // Xử lý thanh toán tại quầy
    virtual PaymentResult processPayment() override {
        PaymentResult result = CashPayment::processPayment();
        // Chỉ in hóa đơn khi thanh toán thành công
        if (result.success) {
            printReceipt();
        }
        return result;
    }
};

// ============================================================
// PHẦN 5: BANKTRANSFERPAYMENT - CHUYỂN KHOẢN NGÂN HÀNG
// ============================================================

class BankTransferPayment : public Payment {
private:
    string bankName;                // Tên ngân hàng
    string transactionReference;    // Mã tham chiếu
    string accountNumber;           // Số tài khoản nhận tiền

public:
    // Constructor thanh toán chuyển khoản
    BankTransferPayment(string id, string tid, double a,
                        string bank, string ref, string account)
        : Payment(id, tid, a),
          bankName(bank),
          transactionReference(ref),
          accountNumber(account) {}

    // Kiểm tra thông tin chuyển khoản
    virtual bool verifyBankTransfer() {
        return !bankName.empty() &&
               !accountNumber.empty();
    }
    // Kiểm tra mã tham chiếu
    virtual bool checkReference() {
        return !transactionReference.empty();
    }
    // Xử lý thanh toán chuyển khoản
    virtual PaymentResult processPayment() override {
        if (!verifyBankTransfer()) {
            return PaymentResult(false, "Thong tin ngan hang khong hop le");
        }
        if (!checkReference()) {
            return PaymentResult(false, "Ma tham chieu khong hop le");
        }
        // Mô phỏng ngân hàng xác nhận chuyển khoản
        setStatus(PaymentStatus::PAID);

        return PaymentResult(true, "Thanh toan chuyen khoan thanh cong");
    }
    // Hoàn tiền qua chuyển khoản
    virtual bool refundPayment() override {
        return Payment::refundPayment();
    }
    // Hiển thị thông tin chuyển khoản
    virtual void displayPayment() override {
        Payment::displayPayment();
        cout << "Ngan hang: " << bankName << endl;
        cout << "Ma tham chieu: " << transactionReference << endl;
        cout << "So tai khoan: " << accountNumber << endl;
    }
};

// PHẦN 6: QRBANKTRANSFER - THANH TOÁN BẰNG MÃ QR

class QRBankTransfer : public BankTransferPayment {
private:
    string QRcode;          // Mã QR
    string bankAccount;     // Tài khoản nhận tiền
    string transferContent; // Nội dung chuyển khoản

public:
    // Constructor thanh toán bằng mã QR
    QRBankTransfer(string id, string tid, double a,
                   string qr, string account, string content)
        : BankTransferPayment(id, tid, a, "QR Bank", qr, account),
          QRcode(qr), bankAccount(account),
          transferContent(content) {}

    // Tạo mã QR thanh toán
    string generateQRCode() {
        return QRcode;
    }
    // Kiểm tra mã QR
    bool scanQRCode() {
        return !QRcode.empty();
    }
    // Xác minh giao dịch QR
    bool verifyQRTransaction() {
        return scanQRCode() &&
               !bankAccount.empty() &&
               !transferContent.empty();
    }
    // Xử lý thanh toán QR
    virtual PaymentResult processPayment() override {
        if (!verifyQRTransaction()) {
            return PaymentResult(false, "Thong tin QR khong hop le");
        }
        // Mô phỏng xác nhận giao dịch QR
        setStatus(PaymentStatus::PAID);
        return PaymentResult(true, "Thanh toan QR thanh cong");
    }
};

// PHẦN 7: EWALLETPAYMENT - THANH TOÁN VÍ ĐIỆN TỬ

class EWalletPayment : public Payment {
private:
    string walletProvider;  // Nhà cung cấp ví
    string walletID;        // Mã tài khoản ví
    string transactionCode; // Mã giao dịch ví

public:
    // Constructor thanh toán ví điện tử
    EWalletPayment(string id, string tid, double a,
                   string provider, string wid)
        : Payment(id, tid, a),
          walletProvider(provider),
          walletID(wid),
          transactionCode("") {}

    // Kiểm tra thông tin ví điện tử
    virtual bool verifyWallet() {
        return !walletProvider.empty() &&
               !walletID.empty();
    }
    // Xác nhận giao dịch ví điện tử
    virtual bool confirmTransaction() {
        return !transactionCode.empty();
    }
    // Cập nhật mã giao dịch ví
    void setTransactionCode(string code) {
        transactionCode = code;
    }

    // Xử lý thanh toán ví điện tử
    virtual PaymentResult processPayment() override {
        if (!verifyWallet()) {
            return PaymentResult(false, "Thong tin vi dien tu khong hop le");
        }
        if (!confirmTransaction()) {
            return PaymentResult(false, "Giao dich vi chua duoc xac nhan");
        }
        // Mô phỏng ví điện tử xác nhận thanh toán
        setStatus(PaymentStatus::PAID);
        return PaymentResult(true, "Thanh toan vi dien tu thanh cong");
    }

    // Hoàn tiền qua ví điện tử
    virtual bool refundPayment() override {
        return Payment::refundPayment();
    }

    // Hiển thị thông tin ví điện tử
    virtual void displayPayment() override {
        Payment::displayPayment();
        cout << "Nha cung cap vi: " << walletProvider << endl;
        cout << "Ma vi: " << walletID << endl;
        cout << "Ma giao dich vi: " << transactionCode << endl;
    }
};

// PHẦN 8: MOMOPAYMENT - THANH TOÁN MOMO

class MomoPayment : public EWalletPayment {
private:
    string momoAccount;         // Tài khoản MoMo
    string momoTransactionID;   // Mã giao dịch MoMo
    string momoStatus;          // Trạng thái MoMo

public:
    // Constructor thanh toán MoMo
    MomoPayment(string id, string tid, double a,
                string account, string mtid)
        : EWalletPayment(id, tid, a, "MoMo", account),
          momoAccount(account),
          momoTransactionID(mtid),
          momoStatus("UNPAID") {

        // Gán mã giao dịch MoMo cho lớp cha
        setTransactionCode(mtid);
    }
    // Kiểm tra kết nối tài khoản MoMo
    bool connectMomoAccount() {
        return !momoAccount.empty();
    }
    // Xác minh tài khoản MoMo
    bool verifyMomoAccount() {
        return connectMomoAccount();
    }
    // Xác nhận giao dịch MoMo
    bool confirmMomoTransaction() {
        return !momoTransactionID.empty();
    }

    // Xử lý thanh toán MoMo
    virtual PaymentResult processPayment() override {
        if (!verifyMomoAccount()) {
            return PaymentResult(false, "Tai khoan MoMo khong hop le");
        }
        if (!confirmMomoTransaction()) {
            return PaymentResult(false, "Ma giao dich MoMo khong hop le");
        }
        momoStatus = "PAID";
        setStatus(PaymentStatus::PAID);
        return PaymentResult(true, "Thanh toan MoMo thanh cong");
    }
};

// PHẦN 9: ZALOPAYPAYMENT - THANH TOÁN ZALOPAY

class ZaloPayPayment : public EWalletPayment {
private:
    string zaloPayAccount;          // Tài khoản ZaloPay
    string zaloPayTransactionID;    // Mã giao dịch ZaloPay
    string zaloPayStatus;           // Trạng thái ZaloPay

public:
    // Constructor thanh toán ZaloPay
    ZaloPayPayment(string id, string tid, double a,
                   string account, string ztid)
        : EWalletPayment(id, tid, a, "ZaloPay", account),
          zaloPayAccount(account),
          zaloPayTransactionID(ztid),
          zaloPayStatus("UNPAID") {
        // Gán mã giao dịch ZaloPay cho lớp cha
        setTransactionCode(ztid);
    }
    // Kiểm tra kết nối tài khoản ZaloPay
    bool connectZaloPayAccount() {
        return !zaloPayAccount.empty();
    }
    // Xác minh tài khoản ZaloPay
    bool verifyZaloPayAccount() {
        return connectZaloPayAccount();
    }
    // Xác nhận giao dịch ZaloPay
    bool confirmZaloPayTransaction() {
        return !zaloPayTransactionID.empty();
    }

    // Xử lý thanh toán ZaloPay
    virtual PaymentResult processPayment() override {
        if (!verifyZaloPayAccount()) {
            return PaymentResult(false, "Tai khoan ZaloPay khong hop le");
        }

        if (!confirmZaloPayTransaction()) {
            return PaymentResult(false, "Ma giao dich ZaloPay khong hop le");
        }

        zaloPayStatus = "PAID";
        setStatus(PaymentStatus::PAID);
        return PaymentResult(true, "Thanh toan ZaloPay thanh cong");
    }
};

// PHẦN 10: FEECALCULATOR - LỚP TÍNH PHÍ VÀ VAT

class FeeCalculator {
private:
    double vatRate;    // Tỷ lệ VAT, mặc định 8%

public:
    // Constructor khởi tạo tỷ lệ VAT mặc định 8%
    FeeCalculator() {
        vatRate = 0.08;
    }
    // Lấy tỷ lệ VAT hiện tại
    double getVATRate() const {
        return vatRate;
    }
    // Cập nhật tỷ lệ VAT
    void setVATRate(double rate) {
        if (rate >= 0 && rate <= 1) {
            vatRate = rate;
        }
    }
    // Tính tổng phí trước VAT
    // Các khoản phí được truyền từ Transaction
    double calculateSubtotal(double rentalFee,
                             double lateFee,
                             double damageFee) {
        return rentalFee + lateFee + damageFee;
    }
    // Tính VAT trên toàn bộ tổng phí
    double calculateVAT(double subtotal) {
        return subtotal * vatRate;
    }
    // Tính tổng tiền cuối cùng bao gồm VAT
    double calculateFinalPayable(double subtotal) {
        return subtotal + calculateVAT(subtotal);
    }
    // Tính số tiền cần hoàn lại sau khi trừ khoản khấu trừ
    double calculateRefundAmount(double paidAmount,
                                 double deduction) {
        if (paidAmount < 0 || deduction < 0) {
            return 0;
        }
        if (deduction > paidAmount) {
            return 0;
        }
        return paidAmount - deduction;
    }
};

#endif

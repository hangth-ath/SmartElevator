import React, { useEffect, useState } from 'react';
import axios from 'axios';
import { Receipt, DollarSign, CheckCircle2, AlertCircle, CreditCard } from 'lucide-react';
import { Bill } from '../types';

export const BillsView: React.FC = () => {
  const [bills, setBills] = useState<Bill[]>([]);
  const [loading, setLoading] = useState<boolean>(true);

  const fetchBills = () => {
    setLoading(true);
    axios.get('/api/bills').then(res => {
      setBills(res.data);
      setLoading(false);
    }).catch(console.error);
  };

  useEffect(() => {
    fetchBills();
  }, []);

  const handlePayBill = (id: number) => {
    axios.post(`/api/bills/${id}/pay`, { method: 'Chuyển khoản VNPAY QR' })
      .then(() => fetchBills())
      .catch(err => alert('Lỗi thanh toán: ' + err.message));
  };

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
      <div className="glass-panel" style={{ padding: '20px 24px', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
        <div>
          <h3 style={{ fontSize: '1.2rem', fontWeight: 800, color: '#f8fafc' }}>Quản Lý Hóa Đơn & Thanh Toán Dịch Vụ</h3>
          <p style={{ color: '#94a3b8', fontSize: '0.85rem' }}>Quản lý phí quản lý, tiền điện, nước và dịch vụ gửi xe chung cư.</p>
        </div>
      </div>

      <div className="glass-panel" style={{ overflow: 'hidden' }}>
        <table style={{ width: '100%', borderCollapse: 'collapse', textAlign: 'left' }}>
          <thead>
            <tr style={{ background: 'rgba(255,255,255,0.04)', borderBottom: '1px solid rgba(255,255,255,0.08)', color: '#94a3b8', fontSize: '0.8rem', textTransform: 'uppercase' }}>
              <th style={{ padding: '14px 20px' }}>Mã Hóa Đơn</th>
              <th style={{ padding: '14px 20px' }}>Căn Hộ</th>
              <th style={{ padding: '14px 20px' }}>Kỳ Phí</th>
              <th style={{ padding: '14px 20px' }}>Số Tiền (VNĐ)</th>
              <th style={{ padding: '14px 20px' }}>Hạn Thanh Toán</th>
              <th style={{ padding: '14px 20px' }}>Trạng Thái</th>
              <th style={{ padding: '14px 20px', textAlign: 'right' }}>Hành Động</th>
            </tr>
          </thead>
          <tbody>
            {loading ? (
              <tr><td colSpan={7} style={{ textAlign: 'center', padding: '40px', color: '#64748b' }}>Đang tải hóa đơn...</td></tr>
            ) : (
              bills.map(b => (
                <tr key={b.id} style={{ borderBottom: '1px solid rgba(255,255,255,0.05)' }}>
                  <td style={{ padding: '14px 20px', fontWeight: 700, color: '#64748b' }}>#BILL-{b.id}</td>
                  <td style={{ padding: '14px 20px', fontWeight: 700, color: '#06b6d4' }}>{b.apartment}</td>
                  <td style={{ padding: '14px 20px', color: '#f8fafc' }}>{b.period}</td>
                  <td style={{ padding: '14px 20px', fontWeight: 700, color: '#f8fafc' }}>
                    {b.amount.toLocaleString('vi-VN')} đ
                  </td>
                  <td style={{ padding: '14px 20px', color: '#94a3b8' }}>{b.due_date}</td>
                  <td style={{ padding: '14px 20px' }}>
                    {b.status === 'PAID' ? (
                      <span className="badge badge-success"><CheckCircle2 size={12} /> Đã Thanh Toán</span>
                    ) : (
                      <span className="badge badge-danger"><AlertCircle size={12} /> Chưa Thanh Toán</span>
                    )}
                  </td>
                  <td style={{ padding: '14px 20px', textAlign: 'right' }}>
                    {b.status === 'UNPAID' && (
                      <button
                        className="btn btn-primary"
                        style={{ padding: '6px 14px', fontSize: '0.8rem' }}
                        onClick={() => handlePayBill(b.id)}
                      >
                        <CreditCard size={14} /> Thanh Toán QR
                      </button>
                    )}
                  </td>
                </tr>
              ))
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};

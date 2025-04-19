document.getElementById('wifi-form').addEventListener('submit', function(e) {
    e.preventDefault();
    // Gửi dữ liệu WiFi đến ESP32
    document.getElementById('wifi-login').style.display = 'none';
    document.getElementById('user-login').style.display = 'block';
  });
  
  document.getElementById('user-form').addEventListener('submit', function(e) {
    e.preventDefault();
    // Gửi dữ liệu người dùng đến ESP32
    document.getElementById('user-login').style.display = 'none';
    document.getElementById('schedule-page').style.display = 'block';
    document.getElementById('date-display').textContent = new Date().toLocaleDateString();
    loadSchedule(); // gọi hàm lấy dữ liệu từ ESP32
  });
  
  function loadSchedule() {
    // Mô phỏng dữ liệu, thực tế bạn sẽ dùng fetch từ ESP32
    const data = [
      { time: '08:00', activity: 'Học toán' },
      { time: '09:30', activity: 'Thể dục' },
      { time: '11:00', activity: 'Ăn trưa' },
    ];
    const tbody = document.getElementById('schedule-body');
    tbody.innerHTML = '';
    data.forEach(item => {
      const row = `<tr><td>${item.time}</td><td>${item.activity}</td></tr>`;
      tbody.innerHTML += row;
    });
  }
  
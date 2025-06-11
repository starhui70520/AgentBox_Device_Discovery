use local_ip_address::local_ip;
use std::net::{UdpSocket, SocketAddrV4, Ipv4Addr};
use std::time::Duration;
use std::thread;

fn main() {
    // SSDP 多播地址和端口
    let multicast_addr: SocketAddrV4 = "239.255.255.250:1900".parse().unwrap();

    // 创建UDP socket
    let socket = UdpSocket::bind("0.0.0.0:0").expect("无法绑定UDP socket");
    socket.join_multicast_v4(&Ipv4Addr::new(239, 255, 255, 250), &Ipv4Addr::new(0, 0, 0, 0))
        .expect("无法加入多播组");

    // 设置TTL
    socket.set_multicast_ttl_v4(4).expect("无法设置TTL");

    println!("开始广播SSDP消息...");

    loop {
        // 获取本地IP地址并处理可能的错误
        let my_local_ip = match local_ip() {
            Ok(ip) => ip.to_string(),  // 转换为字符串
            Err(e) => {
                eprintln!("无法获取本地IP地址: {}", e);
                return;
            }
        };

        // 准备USN部分，移除IP中的点
        let usn_ip = my_local_ip.replace(".", "");

        // 广播消息
        let message = format!(
            "NOTIFY * HTTP/1.1\r\n\
            HOST: 239.255.255.250:1900\r\n\
            CACHE-CONTROL: max-age=60\r\n\
            LOCATION: http://{}\r\n\
            NT: agentbox:device\r\n\
            NTS: ssdp:alive\r\n\
            SERVER: AgentBox/1.0 UPnP/1.0 Rust/1.0\r\n\
            USN: {}\r\n\
            \r\n",
            my_local_ip, usn_ip
        );

        match socket.send_to(message.as_bytes(), multicast_addr) {
            Ok(_) => println!("已发送广播"),
            Err(e) => eprintln!("发送失败: {}", e),
        }
        thread::sleep(Duration::from_secs(3));
    }
}
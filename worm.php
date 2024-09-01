<?php
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Headers: Content-Type");

$servername = "localhost";
$username = "root";
$password = "";
$dbname = "testv1";

// 클라이언트로부터의 JSON 데이터를 가져옵니다.
$json = file_get_contents('php://input');
$data = json_decode($json);

// MySQL 서버에 연결합니다.
$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
  die("Connection failed: " . $conn->connect_error);
}

// 들어오는 객체를 처리합니다.
foreach($dataArray as $data) {
  $objectClass = $data->objectClass;
  $x = $data->bbox->x;
  $y = $data->bbox->y;

  // 이전 좌표를 데이터베이스에서 가져옵니다.
  $prev_sql = "SELECT coordinatex, coordinatey FROM caterpillar WHERE objectnumber = ?";
  $prev_stmt = $conn->prepare($prev_sql);
  $prev_stmt->bind_param("s", $objectClass); // 객체 고유 번호를 사용하여 이전 좌표를 찾습니다.
  $prev_stmt->execute();
  $prev_result = $prev_stmt->get_result();
  $prev_row = $prev_result->fetch_assoc();
  $previousX = $prev_row["coordinatex"];
  $previousY = $prev_row["coordinatey"];
  $prev_stmt->close();

  // 활동량을 계산합니다.
  $cal = sqrt(pow($x - $previousX, 2) + pow($y - $previousY, 2));
  if($cal > 1 && $cal < 20){
    $activityLevel = sqrt(pow($x - $previousX, 2) + pow($y - $previousY, 2));
  }
  else {
    $activityLevel = 0;
  }

  // 새로운 데이터를 데이터베이스에 저장합니다.
  $sql = "INSERT INTO caterpillar (objectnumber, coordinatex, coordinatey, activitylevel) VALUES (?, ?, ?, ?) ON DUPLICATE KEY UPDATE coordinatex=?, coordinatey=?, activitylevel=?";
  $stmt = $conn->prepare($sql);
  $stmt->bind_param("sssssss", $objectClass, $x, $y, $activityLevel, $x, $y, $activityLevel);
  $stmt->execute();
}

if ($stmt->affected_rows > 0) {
  echo json_encode(array("status" => "success"));
} else {
  echo json_encode(array("status" => "error", "error" => $conn->error));
}

$stmt->close();
$conn->close();
?>

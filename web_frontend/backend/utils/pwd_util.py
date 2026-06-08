from passlib.context import CryptContext

pwd_context = CryptContext(schemes=['bcrypt'], deprecated='auto')

# bcrypt 只接受最多 72 字节，超过会报错；对传入密码按 UTF-8 截断
BCRYPT_MAX_PASSWORD_BYTES = 72


def _truncate_for_bcrypt(s) -> str:
    """确保为 str 且 UTF-8 字节数不超过 bcrypt 限制。"""
    if s is None:
        return ''
    if isinstance(s, bytes):
        s = s.decode('utf-8', errors='replace')
    s = str(s)
    if not s:
        return s
    b = s.encode('utf-8')
    if len(b) <= BCRYPT_MAX_PASSWORD_BYTES:
        return s
    b = b[:BCRYPT_MAX_PASSWORD_BYTES]
    return b.decode('utf-8', errors='ignore')


class PwdUtil:
    """
    密码工具类
    """

    @classmethod
    def verify_password(cls, plain_password, hashed_password):
        """
        工具方法：校验当前输入的密码与数据库存储的密码是否一致

        :param plain_password: 当前输入的密码
        :param hashed_password: 数据库存储的密码
        :return: 校验结果
        """
        if not plain_password:
            return False
        plain = _truncate_for_bcrypt(plain_password)
        try:
            return pwd_context.verify(plain, hashed_password)
        except ValueError as e:
            # bcrypt 5.x 对超长密码抛错；passlib 内部 detect_wrap_bug 也会用 255 字节测试
            if '72 bytes' in str(e):
                return False
            raise
        except Exception as e:
            # passlib 懒加载后端时可能触发 72 bytes 等错误，避免登录接口 500
            if '72 bytes' in str(e) or 'truncate' in str(e).lower():
                return False
            raise

    @classmethod
    def get_password_hash(cls, input_password):
        """
        工具方法：对当前输入的密码进行加密

        :param input_password: 输入的密码
        :return: 加密成功的密码
        """
        if not input_password:
            return pwd_context.hash('')
        pwd = _truncate_for_bcrypt(input_password)
        return pwd_context.hash(pwd)

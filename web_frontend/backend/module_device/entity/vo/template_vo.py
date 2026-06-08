from pydantic import BaseModel, Field, ConfigDict
from pydantic.alias_generators import to_camel
from pydantic_validation_decorator import NotBlank, Size
from module_device.annotation.pydantic_annotation import as_query
from typing import Optional, Any


class TemplateModel(BaseModel):
    """
    模板模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    id: Optional[int] = Field(default=None, description='ID')
    type: int
    protocol_type: int
    dev_type: Optional[int] = Field(default=None, description='设备类型')
    name: str
    remark: Optional[str]

class TemplateCreateUpdateModel(BaseModel):
    """
    模板创建更新模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    id: Optional[int] = Field(default=None, description='ID')
    type: int
    protocol_type: int
    dev_type: Optional[int] = Field(default=None, description='设备类型')
    name: str
    remark: Optional[str] = Field(default=None, description='备注')

    @NotBlank(field_name='type', message='Template type cannot be empty')
    def get_type(self):
        return self.type

    @NotBlank(field_name='protocol_type', message='Protocol type cannot be empty') 
    def get_protocol_type(self):
        return self.protocol_type

    @NotBlank(field_name='name', message='Template name cannot be empty')
    @Size(field_name='name', min_length=1, max_length=100, message='Template name length must be between 1 and 100 characters')
    def get_name(self):
        return self.name

    def validate_fields(self):
        self.get_type()
        self.get_protocol_type()
        self.get_name()

class DeleteTemplateModel(BaseModel):
    """
    删除模板模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    template_ids: str = Field(description='需要删除的模板ID')

@as_query
class TemplatePageQueryModel(BaseModel):
    """
    模板分页查询模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    page_num: int = Field(default=1, description='当前页数')
    page_size: int = Field(default=10, description='每页显示数量')
    name: Optional[str] = Field(default=None, description='模板名称')
    type: Optional[int] = Field(default=None, description='模板类型')
    protocol_type: Optional[int] = Field(default=None, description='协议类型')
    dev_type: Optional[int] = Field(default=None, description='设备类型')


class CrudResponseModel(BaseModel):
    """
    操作响应模型
    """

    is_success: bool = Field(description='操作是否成功')
    message: str = Field(description='响应信息')
    result: Optional[Any] = Field(default=None, description='响应结果')


class UploadResponseModel(BaseModel):
    """
    上传响应模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    file_name: Optional[str] = Field(default=None, description='新文件映射路径')
    new_file_name: Optional[str] = Field(default=None, description='新文件名称')
    original_filename: Optional[str] = Field(default=None, description='原文件名称')
    url: Optional[str] = Field(default=None, description='新文件url')


class TemplatePointModel(BaseModel):
    """
    模板测点模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    id: Optional[int] = Field(default=None, description='ID')
    template_id: int = Field(description='模板ID')
    protocol_type: int = Field(description='协议类型')
    dev_type: int = Field(default=0, description='设备类型')
    group_type: int = Field(default=0, description='组类型')
    code: int = Field(description='代码')
    name: str = Field(default='', description='名称')
    ename: str = Field(default='', description='英文名称')
    data_type: int = Field(description='数据类型')
    attribute: int = Field(description='属性')
    func: int = Field(description='功能')
    reg_addr: int = Field(description='寄存器地址')
    bit_pos: int = Field(description='位位置')
    bit_num: int = Field(description='位数量')
    endian: int = Field(description='字节序')
    precision: float = Field(description='精度')
    ratio: float = Field(description='比例')
    offset: float = Field(description='偏移')
    is_persisted: int = Field(description='是否持久化')
    storage_interval: int = Field(description='存储间隔')
    mutate_bound: Optional[float] = Field(default=None, description='变异边界')
    default_value: Optional[float] = Field(default=None, description='默认值')
    min_value: Optional[float] = Field(default=None, description='最小值')
    max_value: Optional[float] = Field(default=None, description='最大值')
    unit: Optional[str] = Field(default=None, description='单位')
    is_show: int = Field(description='是否显示')

    @NotBlank(field_name='code', message='Code cannot be empty')
    def get_code(self):
        return self.code

    @NotBlank(field_name='name', message='Name cannot be empty')
    def get_name(self):
        return self.name

    @NotBlank(field_name='ename', message='Ename cannot be empty')
    def get_ename(self):
        return self.ename

    @NotBlank(field_name='data_type', message='Data type cannot be empty')
    def get_data_type(self):
        return self.data_type

    @NotBlank(field_name='attribute', message='Attribute cannot be empty')
    def get_attribute(self):
        return self.attribute

    @NotBlank(field_name='func', message='Function cannot be empty')
    def get_func(self):
        return self.func

    @NotBlank(field_name='reg_addr', message='Register address cannot be empty')
    def get_reg_addr(self):
        return self.reg_addr

    @NotBlank(field_name='bit_pos', message='Bit position cannot be empty')
    def get_bit_pos(self):
        return self.bit_pos

    @NotBlank(field_name='bit_num', message='Bit number cannot be empty')
    def get_bit_num(self):
        return self.bit_num

    @NotBlank(field_name='endian', message='Endian cannot be empty')
    def get_endian(self):
        return self.endian

    @NotBlank(field_name='precision', message='Precision cannot be empty')
    def get_precision(self):
        return self.precision

    @NotBlank(field_name='ratio', message='Ratio cannot be empty')
    def get_ratio(self):
        return self.ratio

    @NotBlank(field_name='offset', message='Offset cannot be empty')
    def get_offset(self):
        return self.offset

    @NotBlank(field_name='is_persisted', message='Is persisted cannot be empty')
    def get_is_persisted(self):
        return self.is_persisted

    @NotBlank(field_name='storage_interval', message='Storage interval cannot be empty')
    def get_storage_interval(self):
        return self.storage_interval

    @NotBlank(field_name='is_show', message='Is show cannot be empty')
    def get_is_show(self):
        return self.is_show

    def validate_fields(self):
        self.get_code()
        self.get_name()
        self.get_ename()
        self.get_data_type()
        self.get_attribute()
        self.get_func()
        self.get_reg_addr()
        self.get_bit_pos()
        self.get_bit_num()
        self.get_endian()
        self.get_precision()
        self.get_ratio()
        self.get_offset()
        self.get_is_persisted()
        self.get_storage_interval()
        self.get_is_show()
